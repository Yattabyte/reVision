/* Screen space reflection shader - part 2 - Screen lookup from UV's */
#version 460
#pragma optionNV(fastmath on)
#pragma optionNV(fastprecision on)
#pragma optionNV(ifcvt none)
#pragma optionNV(inline all)
#pragma optionNV(strict on)
#pragma optionNV(unroll all)
#define M_MAX_SPECULAR_EXP 32 

// SSR Variables
const float maxDistance = -1000.0f;
const float numMips = 6.0f;
const float fadeStart = 0.75f;
const float fadeEnd = 1.0f;

// The screen textures
layout (binding = 0) uniform sampler2D ColorMap;
layout (binding = 1) uniform sampler2D ViewNormalMap;
layout (binding = 2) uniform sampler2D SpecularMap;
layout (binding = 3) uniform sampler2D DepthMap;
layout (binding = 5) uniform sampler2D SSRMap;
layout (binding = 6) uniform sampler2D LightMap;

layout (location = 0) in vec2 TexCoord;
layout (location = 1) flat in mat4 CamPInverse;
layout (location = 5) flat in mat4 CamVInverse;
layout (location = 9) flat in vec3 CamEyePosition;
layout (location = 10) flat in vec2 CamDimensions;

layout (location = 0) out vec4 LightingColor;

// Use PBR lighting methods
#package "lighting_pbr"

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
    const float SpecularPower	= roughnessToSpecularPower(Roughness);    
	
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
	const vec2 cameraDimensions	= CamDimensions;
	const float maxDimension	= max(cameraDimensions.x, cameraDimensions.y);
	float oppositeLength, incircleSize, mipChannel;
	vec2 samplePos;
	vec4 newColor;
    // cone-tracing using an isosceles triangle to approximate a cone in screen space
    for (uint i = 0; i < 14; ++i) {
        // intersection length is the adjacent side, get the opposite side using trig
        oppositeLength			= isoscelesTriangleOpposite(adjacentLength, coneTheta);
         
        // calculate in-radius of the isosceles triangle
        incircleSize			= isoscelesTriangleInRadius(oppositeLength, adjacentLength);
         
        // get the sample position in screen space
        samplePos		 		= SSPos.xy + adjacentUnit * (adjacentLength - incircleSize);
         
        // convert the in-radius into screen size then check what power N to raise 2 to reach it - that power N becomes mip level to sample from
        mipChannel				= clamp(log2(incircleSize * maxDimension), 0.0f, maxMipLevel);
         
        /*
        * Read color and accumulate it using trilinear filtering and weight it.
        * Uses pre-convolved image (color buffer) and glossiness to weigh color contributions.
        * Visibility is accumulated in the alpha channel. Break if visibility is 100% or greater (>= 1.0f).
        */
        newColor				= coneSampleWeightedColor(samplePos, mipChannel, glossMult);
         
        remainingAlpha		   -= newColor.a;
        if (remainingAlpha < 0.0f)
            newColor.rgb       *= (1.0f - abs(remainingAlpha));
    
        totalColor			   += newColor;
         
        if (totalColor.a >= 1.0f)
            break;
        
        adjacentLength		   = isoscelesTriangleNextAdjacent(adjacentLength, incircleSize);
        glossMult    		  *= gloss;
    }
	
	return totalColor.rgb;	
}

vec3 Fresnel_Schlick_Roughness(vec3 f0, float AdotB, float roughness)
{
    return 								f0 + (max(vec3(1.0 - roughness), f0) - f0) * pow(1.0 - AdotB, 5.0);
}   

void main(void)
{   	
	ViewData data;
	GetFragmentData(TexCoord, data);
	// Quit Early
	if (data.View_Depth	>= 1.0f) 		discard;
	
	// SSR Texture
	const vec4 SSRtexture				= texture(SSRMap, TexCoord);		
	const vec3 ReflectionUVS 			= SSRtexture.xyz;
	const float rDotV					= SSRtexture.w;
	if (rDotV <= 0.0f)					discard;
	
	// Variables
	const vec3 SSPos	 				= vec3(TexCoord, data.View_Depth);	
    const vec3 WorldNormal 				= normalize((CamVInverse * vec4(data.View_Normal, 0))).xyz;
	const vec3 CameraVector 			= normalize(data.World_Pos.xyz - CamEyePosition);
	const vec3 ReflectionVector 		= reflect(CameraVector, WorldNormal);
	float Alpha 						= 1.0f;
	const vec3 Reflection				= AcquireSpecular(SSPos, data.Roughness, ReflectionUVS.xy, Alpha);
	
	// Attenuate camera-facing fragments
	const float Atten_Facing			= 1 - smoothstep(0.25, 0.5, dot(-CameraVector, ReflectionVector));	
	// Attenuate perpendicular-facing fragments
	const float Atten_Perpendicular   	= clamp(mix(0.0f, 1.0f, clamp(rDotV * 4.0f, 0.0f, 1.0f)), 0.0f, 1.0f);
	// Attenuate roughness
	const float Atten_Roughness			= clamp(mix(0.0f, 1.0f, (1.0f - data.Roughness) * 4.0f), 0.0f, 1.0f);
	// Attenuate out-of-bounds UV coordinates
	const vec2 UVSamplingAttenuation 	= smoothstep(0.05, 0.1, ReflectionUVS.xy) * (vec2(1.0f) - smoothstep(0.95, 1.0, ReflectionUVS.xy));	
	const float Atten_UV				= UVSamplingAttenuation.x * UVSamplingAttenuation.y;
	// Attenuate back-faces
	const float Atten_BackFace			= smoothstep(-0.17, 0.0, dot(normalize((CamVInverse * vec4(texture(ViewNormalMap, ReflectionUVS.xy).rgb, 0))).xyz, -ReflectionVector));	
	// Attenuate near edge of screen
    const vec2 boundary					= abs(ReflectionUVS.xy - vec2(0.5f, 0.5f)) * 2.0f;
    const float fadeDiffRcp				= 1.0f / (fadeEnd - fadeStart);
    const float Atten_Border			= smoothstep(0.0f, 1.0f, 1.0f - clamp((boundary.x - fadeStart) * fadeDiffRcp, 0.0f, 1.0f) * 1.0f - clamp((boundary.y - fadeStart) * fadeDiffRcp, 0.0f, 1.0f));
	// Attenuate Distance
	const float Atten_Distance  		= 1.0f - clamp(distance(ReflectionUVS, SSPos) / maxDistance, 0.0f, 1.0f);	
	// Final Attenuation
	const float Attenuation				= Atten_Facing * Atten_Perpendicular * Atten_Roughness * Atten_UV * Atten_BackFace * Atten_Border * Atten_Distance * (1.0f - clamp(Alpha, 0.0f, 1.0f));		
	
	LightingColor						= vec4(Reflection, Attenuation);
}