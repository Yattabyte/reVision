/* Point light - lighting shader. */
#version 460
#pragma optionNV(fastmath on)
#pragma optionNV(ifcvt none)
#pragma optionNV(inline all)
#pragma optionNV(strict on)
#pragma optionNV(unroll all)
#define EPSILON 0.00001
#define saturate(value) clamp(value, 0.0f, 1.0f)
layout (early_fragment_tests) in;

layout (binding = 0) uniform sampler2D ColorMap;
layout (binding = 1) uniform sampler2D ViewNormalMap;
layout (binding = 2) uniform sampler2D SpecularMap;
layout (binding = 3) uniform sampler2D DepthMap;
layout (binding = 4) uniform samplerCubeArray ShadowMap;

layout (location = 0) uniform float ShadowSize_Recip;

layout (location = 0) flat in mat4 CamPInverse;
layout (location = 4) flat in mat4 CamVInverse;
layout (location = 8) flat in vec3 CamEyePosition;
layout (location = 9) flat in vec2 CamDimensions;
layout (location = 10) flat in vec3 LightColorInt;
layout (location = 11) flat in vec3 LightPosition;
layout (location = 12) flat in float LightRadius2;
layout (location = 13) flat in int ShadowSpotFinal;

layout (location = 0) out vec3 LightingColor; 

// Use PBR lighting methods
#package "lighting_pbr"

vec2 CalcTexCoord()
{
    return			 				gl_FragCoord.xy / CamDimensions;
}
const vec3 sampleOffsetDirections[20] = vec3[] (
		vec3( 1,  1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1,  1,  1), 
		vec3( 1,  1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1,  1, -1),
		vec3( 1,  1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1,  1,  0),
		vec3( 1,  0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1,  0, -1),
		vec3( 0,  1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0,  1, -1)
	);
#define FactorAmt 1.0 / 20
float CalcShadowFactor(in vec3 LightDirection, in float ViewDistance, in float bias)                                                  
{		
	if (ShadowSpotFinal >= 0) {	
		const float FragmentDepth 	= (length(LightDirection) / LightRadius2) - EPSILON - bias;
		const float diskRadius 		= (1.0 + (ViewDistance / LightRadius2)) * (ShadowSize_Recip * 2);
		
		float Factor = 0.0f, depth;
		vec4 FinalCoord1, FinalCoord2;
		const int Shadowspot1 = ShadowSpotFinal;
		const int Shadowspot2 = ShadowSpotFinal+1;
		for (uint x = 0; x < 20; ++x) {	
			FinalCoord1				= vec4(LightDirection + sampleOffsetDirections[x] * diskRadius, Shadowspot1);
			FinalCoord2				= vec4(FinalCoord1.xyz, Shadowspot2);
			depth 					= min(texture(ShadowMap, FinalCoord1).r, texture(ShadowMap, FinalCoord2).r);
			Factor 			   	 	+= (depth >= FragmentDepth ) ? FactorAmt : 0.0;	
		}
		return 						Factor;
	}
	return 							1.0f;
}   

void main(void)
{		
	// Initialize variables
	ViewData data;
	GetFragmentData(CalcTexCoord(), data);
    if (data.View_Depth >= 1.0f) 	discard; // Discard background fragments	
	const vec3 LightDirection 		= (LightPosition.xyz - data.World_Pos.xyz);
	const vec3 DeltaView 			= CamEyePosition - data.World_Pos.xyz;  
	const float ViewDistance 		= length(DeltaView);
	const vec3 ViewDirection		= DeltaView / ViewDistance;
	const float NdotV 				= dot(data.World_Normal, ViewDirection);
	const float NdotL 		 		= dot(normalize(LightDirection.xyz), data.World_Normal);
	const float NdotL_Clamped		= max(NdotL, 0.0);
	const float NdotV_Clamped		= max(NdotV, 0.0);
	if (NdotL <= 0.f && NdotV <= 0.f) discard; // Discard if light will be zero anyway
	
	// Attenuation	
	const float Distance 			= length(LightDirection);
	const float range 				= 1.0f / LightRadius2;
	const float Attenuation 		= 1.0f - (Distance * Distance) * (range * range);
	if (Attenuation <= 0.0f) 		discard; // Discard if outside of radius
	
	// Shadow
	const float cosAngle			= saturate(1.0f - NdotL);
	const float bias 				= clamp(0.05f * tan(acos(NdotL)), 0.0f, 0.05f);
	const vec3 scaledNormalOffset	= data.World_Normal * (cosAngle * ShadowSize_Recip);
	const float ShadowFactor 		= CalcShadowFactor(-(LightDirection + scaledNormalOffset), ViewDistance, bias);
	if (ShadowFactor <= EPSILON) 	discard; // Discard if completely in shadow
	
	// Direct Light	
	vec3 Fs;
	const vec3 Radiance 			= (LightColorInt * Attenuation) * ShadowFactor;
	const vec3 D_Diffuse			= CalculateDiffuse( data.Albedo );
	const vec3 D_Specular			= BRDF_Specular( data.Roughness, data.Albedo, data.Metalness, data.World_Normal, LightDirection, NdotL_Clamped, NdotV_Clamped, ViewDirection, Fs);
	const vec3 D_Ratio				= (vec3(1.0f) - Fs) * (1.0f - data.Metalness); 
	LightingColor 					= (D_Ratio * D_Diffuse + D_Specular) * Radiance * NdotL_Clamped; 
}
