/* Spot light - lighting shader. */
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
layout (binding = 4) uniform sampler2DArray ShadowMap;

layout (location = 0) uniform float ShadowSize_Recip;

layout (location = 0) flat in mat4 CamPInverse;
layout (location = 4) flat in mat4 CamVInverse;
layout (location = 8) flat in vec3 CamEyePosition;
layout (location = 9) flat in vec2 CamDimensions;
layout (location = 10) flat in vec3 LightColorInt;
layout (location = 11) flat in vec3 LightPosition;
layout (location = 12) flat in vec3 LightVector;
layout (location = 13) flat in float LightRadius2;
layout (location = 14) flat in float LightCutoff;
layout (location = 15) flat in int Shadow_Spot;
layout (location = 16) flat in mat4 ShadowPV;
layout (location = 20) flat in float ShadowIndexFactor;

layout (location = 0) out vec3 LightingColor;       

// Use PBR lighting methods
#package "lighting_pbr"  

vec2 CalcTexCoord()
{
    return			 				gl_FragCoord.xy / CamDimensions;
}

const vec2 sampleOffsetDirections[9] = vec2[] (
	vec2(-1,  -1), vec2(-1,  0), vec2(-1,  1), 
	vec2(0,  -1), vec2(0,  0), vec2(0,  1),
	vec2(1,  -1), vec2(1,  0), vec2(1,  1)
);
#define FactorAmt 1.0 / 9
float CalcShadowFactor(in vec4 LightSpacePos, in float ViewDistance)                                                  
{                                                                                  
	// Bring fragment coordinates from world space into light space, then into texture spaces
	const vec3 ProjCoords 			= LightSpacePos.xyz / LightSpacePos.w;                                  
	const vec2 UVCoords 			= 0.5f * ProjCoords.xy + 0.5f;                                                        
	const float FragmentDepth 		= (0.5f * ProjCoords.z + 0.5f) - EPSILON; 		
	const float diskRadius 			= (1.0 + (ViewDistance / LightRadius2)) * (ShadowSize_Recip * 2);
		
	float Factor = 0.0f, depth;
	const int Shadowspot1 = Shadow_Spot;
	const int Shadowspot2 = Shadow_Spot+1;
	vec3 FinalCoord1, FinalCoord2;
	for (uint x = 0; x < 9; ++x) { 
		FinalCoord1 				= vec3(UVCoords + sampleOffsetDirections[x] * diskRadius, Shadowspot1);			
		FinalCoord2 				= vec3(FinalCoord1.xy, Shadowspot2);
		depth 						= min(texture(ShadowMap, FinalCoord1).r, texture(ShadowMap, FinalCoord2).r);
		Factor 			   			+= (depth >= FragmentDepth) ? FactorAmt : 0.0;
	}		
	return 							Factor * ShadowIndexFactor;
}  

void main(void)
{		
	// Initialize variables
	ViewData data;
	GetFragmentData(CalcTexCoord(), data);
   	if (data.View_Depth >= 1.0f)	discard; // Discard background fragments
	
	// Spot Angle Factor
	const vec3 LightToPixel 		= normalize(data.World_Pos.xyz - LightPosition);                             
	const float SpotFactor 			= dot(LightToPixel, LightVector); 			
    if (SpotFactor < LightCutoff) discard;	// Discard if light falls outside of FOV
	
	const vec3 LightPosDirection 	= -LightToPixel;
	const vec3 DeltaView 			= CamEyePosition - data.World_Pos.xyz;  
	const float ViewDistance 		= length(DeltaView);
	const vec3 ViewDirection		= DeltaView / ViewDistance;
	const float NdotV 				= dot(data.World_Normal, ViewDirection);
	const float NdotL 		 		= dot(normalize(LightPosDirection), data.World_Normal);
	const float NdotL_Clamped		= max(NdotL, 0.0);
	const float NdotV_Clamped		= max(NdotV, 0.0);
	if (NdotL <= 0.f && abs(NdotV) <= 0.f) discard; // Discard if light will be zero anyway
	
	// Attenuation	
	const float Distance 			= length(LightToPixel);
	const float range 				= 1.0f / LightRadius2;
	const float Attenuation 		= 1.0f - (Distance * Distance) * (range * range); 	
	if (Attenuation <= 0.0f) 		discard; // Discard if outside of radius	
	
	// Shadow	
	const float cosAngle			= saturate(1.0f - NdotL);
	const float bias 				= clamp(0.005f * tan(acos(NdotL)), 0.0f, 0.005f);
	const vec4 scaledNormalOffset	= vec4(data.World_Normal * (cosAngle * ShadowSize_Recip), 0);
	const float ShadowFactor 		= CalcShadowFactor(ShadowPV * (data.World_Pos + scaledNormalOffset), ViewDistance); 
	if (ShadowFactor <= EPSILON) 	discard; // Discard if completely in shadow	
	
	// Direct Light
	vec3 Fs;
	const vec3 Radiance 			= (LightColorInt * Attenuation ) * (1.0f - (1.0f - SpotFactor) * 1.0f/(1.0f - LightCutoff)) * ShadowFactor;
	const vec3 D_Diffuse			= CalculateDiffuse( data.Albedo );
	const vec3 D_Specular			= BRDF_Specular( data.Roughness, data.Albedo, data.Metalness, data.World_Normal, LightPosDirection, NdotL_Clamped, NdotV, ViewDirection, Fs);
	const vec3 D_Ratio				= (vec3(1.0f) - Fs) * (1.0f - data.Metalness); 
	LightingColor					= (D_Ratio * D_Diffuse + D_Specular) * Radiance * NdotL_Clamped;
}
