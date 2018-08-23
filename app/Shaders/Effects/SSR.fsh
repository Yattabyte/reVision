#version 460
#pragma optionNV(fastmath on)
#pragma optionNV(fastprecision on)
#pragma optionNV(ifcvt none)
#pragma optionNV(inline all)
#pragma optionNV(strict on)
#pragma optionNV(unroll all)
#define M_MAX_SPECULAR_EXP 32 
#package "lighting_pbr"


// SSR Variables
const float rayStep = 0.01f;
const uint maxSteps = 6u;
const uint maxBinarySteps = 4u;
const float maxDistance = 500.0f;
const float numMips = 6.0f;
const float fadeStart = 0.1f;
const float fadeEnd = 0.9f;

// The screen texture
layout (binding = 4) uniform sampler2D LightMap;
layout (binding = 5) uniform sampler2D EnvironmentBRDF;
layout (binding = 6) uniform sampler2D BayerMatrix;

layout (location = 0) in vec2 TexCoord;
layout (location = 0) out vec4 LightingColor;

vec3 RayMarch(vec3 SSReflectionVector, vec3 SSPos);
vec3 AcquireSpecular(in vec3 SSPos, in float Roughness, in vec2 ReflectionUV, inout float remainingAlpha);

vec3 Fresnel_Schlick_Roughness(vec3 f0, float AdotB, float roughness)
{
    return 								f0 + (max(vec3(1.0 - roughness), f0) - f0) * pow(1.0 - AdotB, 5.0);
}   

vec2 IntegrateBRDF( in float Roughness, in vec3 Normal, in float NoV )
{
	return 								texture(EnvironmentBRDF, vec2(NoV, Roughness)).xy;
}

void main(void)
{   
	LightingColor 						= vec4(0.0f);
	ViewData data;
	GetFragmentData(TexCoord, data);
	// Quit Early
	if (data.View_Depth	 >= 1.0f || !any(bvec3(data.View_Normal))) { discard;return; }
	
	// Variables
	const vec3 SSPos	 				= vec3(TexCoord, data.View_Depth);
    const vec3 WorldNormal 				= normalize((cameraBuffer.vMatrix_Inverse * vec4(data.View_Normal, 0))).xyz;
	const vec3 CameraVector 			= normalize(data.World_Pos.xyz - cameraBuffer.EyePosition);
	const vec3 ReflectionVector 		= reflect(CameraVector, WorldNormal);
	const vec3 ViewPos_Unit				= normalize(data.View_Pos.xyz); 	
	const vec3 reflected 				= normalize(reflect(ViewPos_Unit, normalize(data.View_Normal))); 
	const vec4 PointAlongReflectionVec 	= vec4(maxDistance * ReflectionVector + data.World_Pos.xyz, 1.0f);
	vec4 SSReflectionPoint				= cameraBuffer.pMatrix * cameraBuffer.vMatrix * PointAlongReflectionVec;
	SSReflectionPoint 					/= SSReflectionPoint.w;
	SSReflectionPoint.xy 				= SSReflectionPoint.xy * 0.5 + 0.5;
	const vec3 SSReflectionVector		= normalize(SSReflectionPoint.xyz - SSPos.xyz);
	
	// Ray March
	float Alpha 						= 1.0f;
	const vec3 ReflectionUVS 			= RayMarch(SSReflectionVector, SSPos.xyz);
	const vec3 Reflection				= AcquireSpecular(SSPos, data.Roughness, ReflectionUVS.xy, Alpha);
	
	// Attenuate camera-facing fragments
	const float Atten_Facing			= 1 - smoothstep(0.25, 0.5, dot(-CameraVector, ReflectionVector));	
	// Attenuate perpendicular-facing fragments
	const float Atten_Perpendicular   	= clamp(mix(0.0f, 1.0f, clamp(dot(reflected, ViewPos_Unit) * 4.0f, 0.0f, 1.0f)), 0.0f, 1.0f);
	// Attenuate roughness
	const float Atten_Roughness			= clamp(mix(0.0f, 1.0f, (1.0f - data.Roughness) * 4.0f), 0.0f, 1.0f);
	// Attenuate out-of-bounds UV coordinates
	const vec2 UVSamplingAttenuation 	= smoothstep(0.05, 0.1, ReflectionUVS.xy) * (vec2(1.0f) - smoothstep(0.95, 1.0, ReflectionUVS.xy));	
	const float Atten_UV				= UVSamplingAttenuation.x * UVSamplingAttenuation.y;
	// Attenuate back-faces
	const float Atten_BackFace			= smoothstep(-0.17, 0.0, dot(normalize((cameraBuffer.vMatrix_Inverse * vec4(texture(ViewNormalMap, ReflectionUVS.xy).rgb, 0))).xyz, -ReflectionVector));	
	// Attenuate near edge of screen
    const vec2 boundary					= abs(ReflectionUVS.xy - vec2(0.5f, 0.5f)) * 2.0f;
    const float fadeDiffRcp				= 1.0f / (fadeEnd - fadeStart);
    const float Atten_Border			= smoothstep(0.0f, 1.0f, 1.0f - clamp((boundary.x - fadeStart) * fadeDiffRcp, 0.0f, 1.0f) * 1.0f - clamp((boundary.y - fadeStart) * fadeDiffRcp, 0.0f, 1.0f));
	// Attenuate Distance
	const float Atten_Distance  		= 1.0f - clamp(distance(ReflectionUVS, SSPos) / maxDistance, 0.0f, 1.0f);	
	// Final Attenuation
	const float Attenuation				= Atten_Facing * Atten_Perpendicular * Atten_Roughness * Atten_UV * Atten_BackFace * Atten_Border * Atten_Distance * (1.0f - clamp(Alpha, 0.0f, 1.0f));
	
	// Final lighting color	
	const vec3 View_Direction			= normalize(cameraBuffer.EyePosition - data.World_Pos.xyz);		
	const float NdotV					= max(dot(data.World_Normal, View_Direction), 0.0);		
	const vec3 F0						= mix(vec3(0.03f), data.Albedo, data.Metalness);
	const vec3 Fs						= Fresnel_Schlick_Roughness(F0, NdotV, data.Roughness);
	const vec2 I_BRDF					= IntegrateBRDF(data.Roughness, data.World_Normal, NdotV);
	const vec3 I_Diffuse				= (data.Albedo / M_PI);
	const vec3 I_Ratio					= (vec3(1.0f) - Fs) * (1.0f - data.Metalness);
	const vec3 I_Specular				= Reflection * (Fs * I_BRDF.x + I_BRDF.y);
	LightingColor 						= vec4((I_Ratio * I_Diffuse + I_Specular) * data.View_AO, Attenuation);
}

vec3 RayMarch(vec3 SSReflectionVector, vec3 SSPos) 
{
	vec3 raySample, prevRaySample, minRaySample, maxRaySample, midRaySample;
	float depthValue;
	const float DitherOffset = texelFetch(BayerMatrix, ivec2(TexCoord * cameraBuffer.CameraDimensions), 0).r ;
	for (uint i = 0; i < maxSteps ; ++i) {	
		prevRaySample = raySample;
        raySample = (i * rayStep) * SSReflectionVector + SSPos + DitherOffset;
		depthValue = texture(DepthMap, raySample.xy).r;
		
		if (raySample.z > depthValue) {
			minRaySample = prevRaySample;
			maxRaySample = raySample;
			midRaySample;
			for (uint i = 0; i < maxBinarySteps; ++i)	{
				midRaySample = mix(minRaySample, maxRaySample, 0.5);
				depthValue = texture(DepthMap, midRaySample.xy).r;
				
				if (midRaySample.z > depthValue)
					maxRaySample = midRaySample;
				else
					minRaySample = midRaySample;
			}
			return midRaySample;
		}		
    } 		
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
    vec3 sampleColor     = textureLod(LightMap, samplePos, mipChannel).rgb;
    return                 vec4(sampleColor * gloss, gloss);
}
 
float isoscelesTriangleNextAdjacent(in float adjacentLength, in float incircleRadius)
{
    // subtract the diameter of the incircle to get the adjacent side of the next level on the cone
    return                 adjacentLength - (incircleRadius * 2.0f);
}

vec3 AcquireSpecular(in vec3 SSPos, in float Roughness, in vec2 ReflectionUV, inout float remainingAlpha)
{ 
    float SpecularPower         = roughnessToSpecularPower(Roughness);    
	
	// convert to cone angle (maximum extent of the specular lobe aperture)
    // only want half the full cone angle since we're slicing the isosceles triangle in half to get a right triangle	
    const float coneTheta       = specularPowerToConeAngle(SpecularPower) * 0.5f;
    const vec2 deltaP           = ReflectionUV - SSPos.xy;
    float adjacentLength        = length(deltaP);
    const vec2 adjacentUnit     = normalize(deltaP);    
    vec4 totalColor             = vec4(0.0f);
    const float maxMipLevel     = numMips - 1.0f;
    const float gloss			= 1.0F - Roughness;
    float glossMult             = gloss;
	const vec2 cameraDimensions	= cameraBuffer.CameraDimensions;
	const float maxDimension	= max(cameraDimensions.x, cameraDimensions.y);
    // cone-tracing using an isosceles triangle to approximate a cone in screen space
    for(uint i = 0; i < 14; ++i) {
        // intersection length is the adjacent side, get the opposite side using trig
        float oppositeLength	= isoscelesTriangleOpposite(adjacentLength, coneTheta);
         
        // calculate in-radius of the isosceles triangle
        float incircleSize		= isoscelesTriangleInRadius(oppositeLength, adjacentLength);
         
        // get the sample position in screen space
        vec2 samplePos		 	= SSPos.xy + adjacentUnit * (adjacentLength - incircleSize);
         
        // convert the in-radius into screen size then check what power N to raise 2 to reach it - that power N becomes mip level to sample from
        float mipChannel		= clamp(log2(incircleSize * maxDimension), 0.0f, maxMipLevel);
         
        /*
        * Read color and accumulate it using trilinear filtering and weight it.
        * Uses pre-convolved image (color buffer) and glossiness to weigh color contributions.
        * Visibility is accumulated in the alpha channel. Break if visibility is 100% or greater (>= 1.0f).
        */
        vec4 newColor			= coneSampleWeightedColor(samplePos, mipChannel, glossMult);
         
        remainingAlpha		   -= newColor.a;
        if(remainingAlpha < 0.0f)
            newColor.rgb       *= (1.0f - abs(remainingAlpha));
    
        totalColor			   += newColor;
         
        if(totalColor.a >= 1.0f)
            break;
        
        adjacentLength		   = isoscelesTriangleNextAdjacent(adjacentLength, incircleSize);
        glossMult    		  *= gloss;
    }
	
	return totalColor.rgb;	
}