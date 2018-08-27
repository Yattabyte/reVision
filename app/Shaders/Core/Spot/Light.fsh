#version 460
#pragma optionNV(fastmath on)
#pragma optionNV(ifcvt none)
#pragma optionNV(inline all)
#pragma optionNV(strict on)
#pragma optionNV(unroll all)
#define EPSILON 0.00001
#define saturate(value) clamp(value, 0.0f, 1.0f)
#package "lighting_pbr"
layout (early_fragment_tests) in;

struct Light_Struct {
	mat4 mMatrix;
	vec4 LightColor;
	vec4 LightPosition;
	vec4 LightVector;
	float LightIntensity;
	float LightRadius;
	float LightCutoff;
};
struct Shadow_Struct {	
	mat4 lightV;
	mat4 lightPV;
	mat4 InverseLightPV;	
	int Shadow_Spot;	
};

layout (std430, binding = 8) readonly buffer Light_Buffer {
	Light_Struct lightBuffers[];
};
layout (std430, binding = 9) readonly buffer Shadow_Buffer {
	Shadow_Struct shadowBuffers[];
};

layout (location = 0) flat in uint LightIndex;
layout (location = 1) flat in uint ShadowIndex;
layout (binding = 4) uniform sampler2DArray ShadowMap;
layout (location = 0) uniform float ShadowSize_Recip;
layout (location = 0) out vec3 LightingColor;         

vec2 CalcTexCoord()
{
    return			 				gl_FragCoord.xy / cameraBuffer.CameraDimensions;
}

const vec2 sampleOffsetDirections[9] = vec2[] (
	vec2(-1,  -1), vec2(-1,  0), vec2(-1,  1), 
	vec2(0,  -1), vec2(0,  0), vec2(0,  1),
	vec2(1,  -1), vec2(1,  0), vec2(1,  1)
);
#define FactorAmt 1.0 / 9
float CalcShadowFactor(in vec4 LightSpacePos, in float ViewDistance, in float RadiusSquared)                                                  
{                                                                                  
	// Bring fragment coordinates from world space into light space, then into texture spaces
	const vec3 ProjCoords 			= LightSpacePos.xyz / LightSpacePos.w;                                  
	const vec2 UVCoords 			= 0.5f * ProjCoords.xy + 0.5f;                                                        
	const float FragmentDepth 		= (0.5f * ProjCoords.z + 0.5f) - EPSILON; 		
	const float diskRadius 			= (1.0 + (ViewDistance / RadiusSquared)) * (ShadowSize_Recip * 2);
		
	float Factor = 0.0f, depth;
	const int Shadowspot1 = shadowBuffers[ShadowIndex].Shadow_Spot;
	const int Shadowspot2 = Shadowspot1+1;
	vec3 FinalCoord1, FinalCoord2;
	for (uint x = 0; x < 9; ++x) { 
		FinalCoord1 				= vec3(UVCoords + sampleOffsetDirections[x] * diskRadius, Shadowspot1);			
		FinalCoord2 				= vec3(FinalCoord1.xy, Shadowspot2);
		depth 						= min(texture(ShadowMap, FinalCoord1).r, texture(ShadowMap, FinalCoord2).r);
		Factor 			   			+= (depth >= FragmentDepth) ? FactorAmt : 0.0;
	}		
	return 							Factor;
}  

void main(void)
{		
	// Initialize variables
	ViewData data;
	GetFragmentData(CalcTexCoord(), data);
   	if (data.View_Depth >= 1.0f)	discard; // Discard background fragments
	
	// Spot Angle Factor
	const vec3 LightToPixel 		= normalize(data.World_Pos.xyz - lightBuffers[LightIndex].LightPosition.xyz);                             
	const float SpotFactor 			= dot(LightToPixel, lightBuffers[LightIndex].LightVector.xyz); 			
    if (SpotFactor < lightBuffers[LightIndex].LightCutoff) discard;	// Discard if light falls outside of FOV
	
	const vec3 LightPosDirection 	= -LightToPixel;
	const vec3 DeltaView 			= cameraBuffer.EyePosition - data.World_Pos.xyz;  
	const float ViewDistance 		= length(DeltaView);
	const vec3 ViewDirection		= DeltaView / ViewDistance;
	const float NdotV 				= dot(data.World_Normal, ViewDirection);
	const float NdotL 		 		= dot(normalize(LightPosDirection), data.World_Normal);
	const float NdotL_Clamped		= max(NdotL, 0.0);
	const float NdotV_Clamped		= max(NdotV, 0.0);
	if (NdotL <= 0.f && abs(NdotV) <= 0.f) discard; // Discard if light will be zero anyway
	
	// Attenuation	
	const float Distance 			= length(LightToPixel);
	const float	RadiusSquared		= lightBuffers[LightIndex].LightRadius * lightBuffers[LightIndex].LightRadius;
	const float range 				= 1.0f / RadiusSquared;
	const float Attenuation 		= 1.0f - (Distance * Distance) * (range * range); 	
	if (Attenuation <= 0.0f) 		discard; // Discard if outside of radius	
	
	// Shadow	
	const float cosAngle			= saturate(1.0f - NdotL);
	const float bias 				= clamp(0.005f * tan(acos(NdotL)), 0.0f, 0.005f);
	const vec4 scaledNormalOffset	= vec4(data.World_Normal * (cosAngle * ShadowSize_Recip), 0);
	const float ShadowFactor 		= CalcShadowFactor(shadowBuffers[ShadowIndex].lightPV * (data.World_Pos + scaledNormalOffset), ViewDistance, RadiusSquared); 
	if (ShadowFactor <= EPSILON) 	discard; // Discard if completely in shadow
	
	
	// Direct Light
	vec3 Fs;
	const vec3 Radiance 			= ( (lightBuffers[LightIndex].LightColor.xyz * lightBuffers[LightIndex].LightIntensity) * Attenuation ) * (1.0f - (1.0f - SpotFactor) * 1.0f/(1.0f - lightBuffers[LightIndex].LightCutoff)) * ShadowFactor;
	const vec3 D_Diffuse			= CalculateDiffuse( data.Albedo );
	const vec3 D_Specular			= BRDF_Specular( data.Roughness, data.Albedo, data.Metalness, data.World_Normal, LightPosDirection, NdotL_Clamped, NdotV, ViewDirection, Fs);
	const vec3 D_Ratio				= (vec3(1.0f) - Fs) * (1.0f - data.Metalness); 
	LightingColor					= (D_Ratio * D_Diffuse + D_Specular) * Radiance * NdotL_Clamped;
}
