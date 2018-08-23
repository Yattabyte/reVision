#version 460
#define M_MAX_SPECULAR_EXP 32 
#package "lighting_pbr"

// SSR Variables
layout (location = 0) uniform float rayStep = 0.1f;
layout (location = 1) uniform float minRayStep = 0.1f;
layout (location = 2) uniform uint maxSteps = 14u;
layout (location = 3) uniform uint numBinarySearchSteps = 4u;
layout (location = 4) uniform float maxDistance = 50.0f;
layout (location = 5) uniform float numMips = 1.0f;
layout (location = 6) uniform float fadeStart = 0.4f;
layout (location = 7) uniform float fadeEnd = 0.9f;

// The screen texture
layout (binding = 4) uniform sampler2D LightMap;

layout (location = 0) in vec2 TexCoord;
layout (location = 0) out vec4 ReflectionColor;

vec3 CalcViewPositionFromDepth(in vec2 TexCoord, in float Depth, in mat4 invPMatrix)
{
    // Combine UV & depth into XY & Z (NDC)
    vec3 rawPosition 				= vec3(TexCoord, Depth);
   
    // Convert from (0, 1) range to (-1, 1)
    vec4 ScreenSpacePosition 		= vec4( rawPosition * 2 - 1, 1);
 
    // Undo Perspective transformation to bring into view space
    vec4 ViewPosition 				= invPMatrix * ScreenSpacePosition;
   
    // Perform perspective divide and return
    return 							ViewPosition.xyz / ViewPosition.w;
}

void CalcPosition(in vec2 TexCoord, out vec3 View, out vec3 World)
{ 
	// Combine UV & depth into XY & Z (NDC)
    vec3 rawPosition 				= vec3(TexCoord, texture(DepthMap, TexCoord).r);
   
    // Convert from (0, 1) range to (-1, 1)
    vec4 ScreenSpacePosition 		= vec4( rawPosition * 2 - 1, 1 );
 
    // Undo transformations
    vec4 ViewPosition 				= cameraBuffer.pMatrix_Inverse * ScreenSpacePosition;
    vec4 WorldPosition 				= cameraBuffer.vMatrix_Inverse * ViewPosition;
   
    // Perform perspective divide and return
    View 							= ViewPosition.xyz / ViewPosition.w;
    World 							= WorldPosition.xyz / WorldPosition.w;
}

vec3 BinarySearch(vec3 dir, inout vec3 hitCoord, out float dDepth, in mat4 pMatrix, in mat4 invPMatrix)
{ 
	float depth;
	vec4 projectedCoord, ViewPosition;
	
    for(uint i = 0; i < 5; ++i) {
        projectedCoord 			= pMatrix * vec4(hitCoord, 1.0);
        projectedCoord.xy 	   	/= projectedCoord.w;
        projectedCoord.xy 		= projectedCoord.xy * 0.5 + 0.5;
  
		ViewPosition 			= invPMatrix * vec4(vec3(projectedCoord.xy, texture(DepthMap, projectedCoord.xy).r) * 2 - 1, 1);	   
		depth					= ViewPosition.z / ViewPosition.w;
        dDepth 					= hitCoord.z - depth; 
 
        dir 				   	*= 0.5;		
        if(dDepth > 0.0)
            hitCoord 		   	+= dir; 
		else 
			hitCoord 			-= dir;    
    } 
 
    projectedCoord 				= pMatrix * vec4(hitCoord, 1.0);
    projectedCoord.xy 	   	   /= projectedCoord.w;
    projectedCoord.xy 			= projectedCoord.xy * 0.5 + 0.5;
 
    return vec3(projectedCoord.xy, depth);
}

vec3 RayCast(vec3 dir, inout vec3 hitCoord, out float dDepth)
{
	const mat4 pMatrix = cameraBuffer.pMatrix;
	const mat4 invPMatrix = cameraBuffer.pMatrix_Inverse;
	float depth;
	vec4 projectedCoord, ViewPosition;
    dir	 					   *= rayStep;
    for (uint i = 0; i < maxSteps ; ++i) {
        hitCoord 			   += dir; 
 
        projectedCoord 			= pMatrix * vec4(hitCoord, 1.0);
        projectedCoord.xy 	   /= projectedCoord.w;
        projectedCoord.xy 		= projectedCoord.xy * 0.5 + 0.5;

		ViewPosition 			= invPMatrix * vec4( vec3(projectedCoord.xy, texture(DepthMap, projectedCoord.xy).r) * 2 - 1, 1);	   
		depth					= ViewPosition.z / ViewPosition.w;
		dDepth 					= hitCoord.z - depth; 
 
        if (dDepth < 0.0)
            return BinarySearch(dir, hitCoord, depth, pMatrix, invPMatrix);
    }
  
    return vec3(0.0f);
}

// based on phong distribution model
float specularPowerToConeAngle(in float specularPower)
{
    if (specularPower >= exp2(M_MAX_SPECULAR_EXP))
        return             0.0f;
    
    const float xi         = 0.244f;
    float exponent         = 1.0f / (specularPower + 1.0f);
    return                 acos(pow(xi, exponent));
}

float roughnessToSpecularPower(in float roughness)
{
    return                 2.0f / (pow(roughness, 4.0f)) - 2.0f;
}
 
// simple trig and algebra - soh, cah, toa - tan(theta) = opp/adj, opp = tan(theta) * adj, then multiply * 2.0f for isosceles triangle base
float isoscelesTriangleOpposite(in float adjacentLength, in float coneTheta)
{
    return                 2.0f * tan(coneTheta) * adjacentLength;
}
 
float isoscelesTriangleInRadius(in float a, in float h)
{
    float a2              = a * a;
    float fh2             = 4.0f * h * h;
    return                (a * (sqrt(a2 + fh2) - a)) / (4.0f * h);
}
 
vec4 coneSampleWeightedColor(in vec2 samplePos, in float mipChannel, in float gloss)
{
    vec3 sampleColor     = textureLod(LightMap, samplePos, 0).rgb;
    return                 vec4(sampleColor * gloss, gloss);
}
 
float isoscelesTriangleNextAdjacent(in float adjacentLength, in float incircleRadius)
{
    // subtract the diameter of the incircle to get the adjacent side of the next level on the cone
    return                 adjacentLength - (incircleRadius * 2.0f);
}

vec3 AcquireSpecular(in vec2 TexCoord, in float View_Depth, in float Roughness, in float Gloss, in vec2 ReflectionUV, inout float remainingAlpha)
{ 
	vec3 Screen_Position        = vec3(TexCoord, View_Depth);   
    float SpecularPower         = roughnessToSpecularPower(Roughness);    
	
	// convert to cone angle (maximum extent of the specular lobe aperture)
    // only want half the full cone angle since we're slicing the isosceles triangle in half to get a right triangle	
    // cone-tracing using an isosceles triangle to approximate a cone in screen space
    float coneTheta             = specularPowerToConeAngle(SpecularPower) * 0.5f;
    vec2 deltaP                 = ReflectionUV - Screen_Position.xy;
    float adjacentLength        = length(deltaP);
    vec2 adjacentUnit           = normalize(deltaP);    
    vec4 totalColor             = vec4(0.0f);
    float maxMipLevel           = float(numMips) - 1.0f;
    float GlossMult             = Gloss;
	const vec2 cameraDimensions	= cameraBuffer.CameraDimensions;
	float maxDimension			= max(cameraDimensions.x, cameraDimensions.y);
    for(int i = 0; i < 14; ++i) {
        // intersection length is the adjacent side, get the opposite side using trig
        float oppositeLength	= isoscelesTriangleOpposite(adjacentLength, coneTheta);
         
        // calculate in-radius of the isosceles triangle
        float incircleSize		= isoscelesTriangleInRadius(oppositeLength, adjacentLength);
         
        // get the sample position in screen space
        vec2 samplePos		 	= Screen_Position.xy + adjacentUnit * (adjacentLength - incircleSize);
         
        // convert the in-radius into screen size then check what power N to raise 2 to reach it - that power N becomes mip level to sample from
        float mipChannel		= clamp(log2(incircleSize * maxDimension), 0.0f, maxMipLevel);
         
        /*
        * Read color and accumulate it using trilinear filtering and weight it.
        * Uses pre-convolved image (color buffer) and glossiness to weigh color contributions.
        * Visibility is accumulated in the alpha channel. Break if visibility is 100% or greater (>= 1.0f).
        */
        vec4 newColor			= coneSampleWeightedColor(samplePos, mipChannel, GlossMult);
         
        remainingAlpha		   -= newColor.a;
        if(remainingAlpha < 0.0f)
            newColor.rgb       *= (1.0f - abs(remainingAlpha));
    
        totalColor			   += newColor;
         
        if(totalColor.a >= 1.0f)
            break;
        
        adjacentLength		   = isoscelesTriangleNextAdjacent(adjacentLength, incircleSize);
        GlossMult    		  *= Gloss;
    }
	
	return totalColor.rgb;	
}

float CalculateFade( in vec3 View_Pos, in float remainingAlpha, in float Gloss, in vec2 ReflectionUV, in float ReflectionDepth, in float RdotV)
{    
    // fade rays close to screen edge
    vec2 boundary				= abs(ReflectionUV - vec2(0.5f, 0.5f)) * 2.0f;
    const float fadeDiffRcp		= 1.0f / (fadeEnd - fadeStart);
    float fadeOnBorder			= 1.0f - clamp((boundary.x - fadeStart) * fadeDiffRcp, 0.0f, 1.0f);
    fadeOnBorder			   *= 1.0f - clamp((boundary.y - fadeStart) * fadeDiffRcp, 0.0f, 1.0f);
    fadeOnBorder              	= smoothstep(0.0f, 1.0f, fadeOnBorder);
    vec3 rayHitPositionVS      	= CalcViewPositionFromDepth(ReflectionUV, ReflectionDepth, cameraBuffer.pMatrix_Inverse);
    float fadeOnDistance   		= 1.0f - clamp(distance(rayHitPositionVS, View_Pos) / maxDistance, 0.0f, 1.0f);
	float fadeOnPerpendicular 	= clamp(RdotV * 4.0f, 0.0f, 1.0f);	
	float fadeOnRoughness 		= clamp(mix(0.0f, 1.0f, Gloss * 4.0f), 0.0f, 1.0f);
    float totalFade         	= fadeOnBorder * fadeOnDistance * fadeOnPerpendicular * fadeOnRoughness * (1.0f - clamp(remainingAlpha, 0.0f, 1.0f));

    return						clamp(totalFade, 0.0f, 1.0f);
}

void main(void)
{   
	ReflectionColor						= vec4(0.0f);
	ViewData data;
	GetFragmentData(TexCoord, data);	
	// Discard background fragments
	if (data.View_Depth >= 1.0f)		discard;	
	// Discard
	if (!any(bvec3(data.View_Normal))) 	discard;	
	// Apply screen space reflection
	const vec3 View_Pos_Unit			= normalize(data.View_Pos.xyz); 	
	const vec3 reflected 				= normalize(reflect(View_Pos_Unit, normalize(data.View_Normal))); 
	const float RdotV               	= max(dot(reflected, View_Pos_Unit), 0.0);
	// Discard backward facing fragments
	if (RdotV < 0.00001F) 				discard;
	// Ray cast
	const vec3 viewPos					= data.View_Pos.xyz;
	vec3 hitPos 						= viewPos;
	float ReflectionDepth       		= 0.0f;
	const vec3 ReflectionCoords			= RayCast(reflected * max(minRayStep, -data.View_Pos.z), hitPos, ReflectionDepth); 		
	const vec2 ReflectionUV				= ReflectionCoords.xy;
	// Don't sample out of bounds
	if (ReflectionUV.x > 1.0f || ReflectionUV.x < 0.0f || ReflectionUV.y > 1.0f || ReflectionUV.y < 0.0f) 			
		discard;
	const float Gloss       			= 1.0f - data.Roughness;
	float Alpha    						= 1.0f;	
	const vec3 SSReflection 			= AcquireSpecular(TexCoord, data.View_Depth, data.Roughness, Gloss, ReflectionUV, Alpha);
	const float Fade					= CalculateFade(data.View_Pos.xyz, Alpha, Gloss, ReflectionUV, ReflectionCoords.z, RdotV);
	ReflectionColor						= vec4(SSReflection, Fade);
}