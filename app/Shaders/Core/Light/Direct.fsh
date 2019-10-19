/* Direct lighting. */
#version 460
#pragma optionNV(fastmath on)
#pragma optionNV(ifcvt none)
#pragma optionNV(inline all)
#pragma optionNV(strict on)
#pragma optionNV(unroll all)
#define MAX_PERSPECTIVE_ARRAY 6
#define EPSILON 0.00001
#define saturate(value) clamp(value, 0.0f, 1.0f)
layout (early_fragment_tests) in;

struct Light_Struct {
	mat4 LightVP[MAX_PERSPECTIVE_ARRAY];
	mat4 mMatrix;
	vec4 LightColor;
	vec4 LightPosition;
	vec4 LightDirection;
	float CascadeEndClipSpace[MAX_PERSPECTIVE_ARRAY];
	float LightIntensity;
	float LightRadius;
	float LightCutoff;
	int Shadow_Spot;
	int Light_Type;
};
layout (std430, binding = 4) readonly buffer Light_Index_Buffer {
	int lightIndexes[];
};
layout (std430, binding = 8) readonly buffer Light_Buffer {
	Light_Struct lightBuffers[];
};

layout (binding = 0) uniform sampler2DArray ColorMap;
layout (binding = 1) uniform sampler2DArray ViewNormalMap;
layout (binding = 2) uniform sampler2DArray SpecularMap;
layout (binding = 3) uniform sampler2DArray DepthMap;
layout (binding = 4) uniform sampler2DArray ShadowMap;

layout (location = 0) uniform float ShadowSize_Recip;

layout (location = 0) flat in mat4 pMatrixInverse;
layout (location = 4) flat in mat4 vMatrixInverse;
layout (location = 8) flat in vec3 EyePosition;
layout (location = 9) flat in vec2 CameraDimensions;
layout (location = 10) flat in int lightIndex;
layout (location = 11) flat in int lightType;

layout (location = 0) out vec3 LightingColor;

// Use PBR lighting methods
#package "lighting_pbr"


const vec2 sampleOffsetDirections[9] = vec2[] (
	vec2(-1,  -1), vec2(-1,  0), vec2(-1,  1), 
	vec2(0,  -1), vec2(0,  0), vec2(0,  1),
	vec2(1,  -1), vec2(1,  0), vec2(1,  1)
);
#define FactorAmt 1.0f / 9.0f
float CalcShadowFactor(in int Index, in vec4 WorldSpacePos, in float bias)                                                  
{     
	// Bring fragment coordinates from world space into light space, then into texture spaces
	const vec4 LightSpacePos				= lightBuffers[lightIndex].LightVP[Index] * WorldSpacePos;
	const vec3 ProjCoords 					= LightSpacePos.xyz / LightSpacePos.w;                                  
	const vec2 UVCoords 					= (0.5f * ProjCoords.xy + 0.5f);                                    
	const float FragmentDepth 				= (0.5f * ProjCoords.z + 0.5f) - EPSILON - bias; 	  
		
	float Factor = 0.0f, depth = 0.0f;
	vec3 FinalCoord;
	for (uint x = 0; x < 9; ++x) { 
		FinalCoord 							= vec3( UVCoords + sampleOffsetDirections[x] * ShadowSize_Recip, lightBuffers[lightIndex].Shadow_Spot + Index );
		depth 								= texture( ShadowMap, FinalCoord ).r;
		Factor 			   		  			+= (depth >= FragmentDepth) ? FactorAmt : 0.0f;	
	}
	return 									Factor;
} 
	
vec2 CalcTexCoord()
{
    return			 						gl_FragCoord.xy / CameraDimensions;
}

void main()
{			
	// Initialize first variables
	ViewData data;
	GetFragmentData(CalcTexCoord(), data);	
	vec3 LightDirection 					= lightBuffers[lightIndex].LightDirection.xyz;	
	if (lightType == 1)
		LightDirection						= lightBuffers[lightIndex].LightPosition.xyz - data.World_Pos.xyz;
	const vec3 ViewDirection				= normalize(EyePosition - data.World_Pos.xyz);
	const float NdotV 						= dot(data.World_Normal, ViewDirection);
	const float NdotL 		 				= dot(normalize(LightDirection.xyz), data.World_Normal);
	const float NdotL_Clamped				= max(NdotL, 0.0);
	const float NdotV_Clamped				= max(NdotV, 0.0);
	
	// Attenuation	
	const float Distance 					= length(LightDirection);
	const float range 						= 1.0f / (lightBuffers[lightIndex].LightRadius * lightBuffers[lightIndex].LightRadius);
	const float Attenuation 				= clamp(1.0f - (Distance * Distance) * (range * range), 0.0f, 1.0f);
	
	// Shadow
	float ShadowFactor						= 1.0f;
	if (lightBuffers[lightIndex].Shadow_Spot >= 0) {
		const float cosAngle				= saturate(1.0f - NdotL);
		const float bias 					= clamp(0.005f * tan(acos(NdotL)), 0.0f, 0.005f);
		const vec4 scaledNormalOffset		= vec4(data.World_Normal * (cosAngle * ShadowSize_Recip), 0);
		for (int index = 0; index < 4; ++index) 
			if (-data.View_Pos.z <= lightBuffers[lightIndex].CascadeEndClipSpace[index]) {
				ShadowFactor 				= CalcShadowFactor(index, (data.World_Pos + scaledNormalOffset), bias);
				break;			
			}		
	}
	
	// Direct Light	
	vec3 Fs;
	const vec3 Radiance 					= lightBuffers[lightIndex].LightColor.xyz * lightBuffers[lightIndex].LightIntensity * Attenuation * ShadowFactor;
	const vec3 D_Diffuse					= CalculateDiffuse( data.Albedo );
	const vec3 D_Specular					= BRDF_Specular( data.Roughness, data.Albedo, data.Metalness, data.World_Normal, LightDirection.xyz, NdotL_Clamped, NdotV_Clamped, ViewDirection, Fs);
	const vec3 D_Ratio						= (vec3(1.0f) - Fs) * (1.0f - data.Metalness);
	LightingColor		 					= vec3(ShadowFactor);//(D_Ratio * D_Diffuse + D_Specular) * Radiance * NdotL_Clamped;
}