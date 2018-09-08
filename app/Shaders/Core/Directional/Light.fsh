/* Directional light - lighting shader. */
#version 460
#pragma optionNV(fastmath on)
#pragma optionNV(ifcvt none)
#pragma optionNV(inline all)
#pragma optionNV(strict on)
#pragma optionNV(unroll all)
#define NUM_CASCADES 4
#define EPSILON 0.00001
#define saturate(value) clamp(value, 0.0f, 1.0f)
layout (early_fragment_tests) in;

layout (binding = 0) uniform sampler2D ColorMap;
layout (binding = 1) uniform sampler2D ViewNormalMap;
layout (binding = 2) uniform sampler2D SpecularMap;
layout (binding = 3) uniform sampler2D DepthMap;
layout (binding = 4) uniform sampler2DArray ShadowMap;

layout (location = 0) uniform float ShadowSize_Recip;

layout (location = 0) in vec2 TexCoord;
layout (location = 1) flat in float shadowIndexFactor;
layout (location = 2) flat in vec3 LightColorInt;
layout (location = 3) flat in vec3 LightDirection;
layout (location = 4) flat in int Shadow_Spot;
layout (location = 5) flat in vec4 CascadeEndClipSpace;
layout (location = 6) flat in mat4 LightVP[NUM_CASCADES];
layout (location = 22) flat in mat4 CamPInverse;
layout (location = 26) flat in mat4 CamVInverse;
layout (location = 30) flat in vec3 CamEyePosition;

layout (location = 0) out vec3 LightingColor;

// Use PBR lighting methods
#package "lighting_pbr"

const vec2 sampleOffsetDirections[9] = vec2[] (
	vec2(-1,  -1), vec2(-1,  0), vec2(-1,  1), 
	vec2(0,  -1), vec2(0,  0), vec2(0,  1),
	vec2(1,  -1), vec2(1,  0), vec2(1,  1)
);
#define FactorAmt 1.0 / 9
float CalcShadowFactor(in int Index, in vec4 LightSpacePos, in float bias)                                                  
{     
	// Bring fragment coordinates from world space into light space, then into texture spaces
	const vec3 ProjCoords 				= LightSpacePos.xyz / LightSpacePos.w;                                  
    const vec2 UVCoords 				= 0.5f * ProjCoords.xy + 0.5f;                                    
    const float FragmentDepth 			= (0.5f * ProjCoords.z + 0.5f) - EPSILON - bias; 	  
	
	float Factor = 0.0f, depth;
	vec3 FinalCoord;
	for (uint x = 0; x < 9; ++x) { 
		FinalCoord 						= vec3( UVCoords + sampleOffsetDirections[x] * ShadowSize_Recip, Shadow_Spot + Index );
		depth 							= texture( ShadowMap, FinalCoord ).r;
		Factor 			   		  	 	+= (depth >= FragmentDepth) ? FactorAmt : 0.0;	
	}
	return 								Factor * shadowIndexFactor;
} 

void main()
{			
	// Initialize first variables
	ViewData data;
	GetFragmentData(TexCoord, data);	
   	if (data.View_Depth >= 1.0f)			discard; // Discard background fragments
	
	const vec3 ViewDirection				= normalize(CamEyePosition - data.World_Pos.xyz);
	const float NdotV 						= dot(data.World_Normal, ViewDirection);
	const float NdotL 		 				= dot(normalize(-LightDirection.xyz), data.World_Normal);
	const float NdotL_Clamped				= max(NdotL, 0.0);
	const float NdotV_Clamped				= max(NdotV, 0.0);
	if (NdotL <= 0.f && abs(NdotV) <= 0.f)	discard; // Discard if light will be zero anyway
	
	// Shadow
	const float cosAngle					= saturate(1.0f - NdotL);
	const float bias 						= clamp(0.005f * tan(acos(NdotL)), 0.0f, 0.005f);
	const vec4 scaledNormalOffset			= vec4(data.World_Normal * (cosAngle * ShadowSize_Recip), 0);
	int index 								= 0;
	for (; index < NUM_CASCADES; ++index) 
		if (-data.View_Pos.z <= CascadeEndClipSpace[index]) 
			break;			
	const vec3 LightPseudoPos				= CamEyePosition + (LightDirection.xyz);
	float ShadowFactor 						= CalcShadowFactor(index, LightVP[index] * (data.World_Pos + scaledNormalOffset), bias);	
	if (ShadowFactor <= EPSILON)			discard; // Discard if completely in shadow
		
	// Direct Light	
	vec3 Fs;
	const vec3 Radiance 					= LightColorInt * ShadowFactor;
	const vec3 D_Diffuse					= CalculateDiffuse( data.Albedo );
	const vec3 D_Specular					= BRDF_Specular( data.Roughness, data.Albedo, data.Metalness, data.World_Normal, -LightDirection.xyz, NdotL_Clamped, NdotV_Clamped, ViewDirection, Fs);
	const vec3 D_Ratio						= (vec3(1.0f) - Fs) * (1.0f - data.Metalness);
	LightingColor		 					= (D_Ratio * D_Diffuse + D_Specular) * Radiance * NdotL_Clamped;
}
