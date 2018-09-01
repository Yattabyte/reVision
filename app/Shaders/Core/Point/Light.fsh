/* Point light - lighting shader. */
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
	float LightIntensity;
	float LightRadius;
};
struct Shadow_Struct {
	mat4 lightV; 
	mat4 lightPV[6];
	mat4 inversePV[6];
	int Shadow_Spot;
};

layout (std430, binding = 8) readonly buffer Light_Buffer {
	Light_Struct lightBuffers[];
};
layout (std430, binding = 9) readonly buffer Shadow_Buffer {
	Shadow_Struct shadowBuffers[];
};

layout (location = 0) flat in int LightIndex;
layout (location = 1) flat in int ShadowIndex;
layout (binding = 4) uniform samplerCubeArray ShadowMap;
layout (location = 0) uniform float ShadowSize_Recip;
layout (location = 0) out vec3 LightingColor; 

vec2 CalcTexCoord()
{
    return			 				gl_FragCoord.xy / cameraBuffer.CameraDimensions;
}
const vec3 sampleOffsetDirections[20] = vec3[] (
		vec3( 1,  1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1,  1,  1), 
		vec3( 1,  1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1,  1, -1),
		vec3( 1,  1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1,  1,  0),
		vec3( 1,  0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1,  0, -1),
		vec3( 0,  1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0,  1, -1)
	);
#define FactorAmt 1.0 / 20
float CalcShadowFactor(in vec3 LightDirection, in float ViewDistance, in float RadiusSquared, in float bias)                                                  
{		
	const float FragmentDepth 		= (length(LightDirection) - bias - EPSILON) / RadiusSquared;
	const float diskRadius 			= (1.0 + (ViewDistance / RadiusSquared)) * (ShadowSize_Recip * 2);
	
	float Factor = 0.0f, depth;
	vec4 FinalCoord1, FinalCoord2;
	const int Shadowspot1 = shadowBuffers[ShadowIndex].Shadow_Spot/6;
	const int Shadowspot2 = Shadowspot1+1;
	for (uint x = 0; x < 20; ++x) {	
		FinalCoord1					= vec4(LightDirection + sampleOffsetDirections[x] * diskRadius, Shadowspot1);
		FinalCoord2					= vec4(FinalCoord1.xyz, Shadowspot2);
		depth 						= min(texture(ShadowMap, FinalCoord1).r, texture(ShadowMap, FinalCoord2).r);
		Factor 			   	 		+= (depth >= FragmentDepth ) ? FactorAmt : 0.0;	
	}
	return 							ShadowIndex != -1 ? Factor : 1.0f;
}   

void main(void)
{		
	// Initialize variables
	ViewData data;
	GetFragmentData(CalcTexCoord(), data);
    if (data.View_Depth >= 1.0f) 	discard; // Discard background fragments	
	const vec3 LightDirection 		= (lightBuffers[LightIndex].LightPosition.xyz - data.World_Pos.xyz);
	const vec3 DeltaView 			= cameraBuffer.EyePosition - data.World_Pos.xyz;  
	const float ViewDistance 		= length(DeltaView);
	const vec3 ViewDirection		= DeltaView / ViewDistance;
	const float NdotV 				= dot(data.World_Normal, ViewDirection);
	const float NdotL 		 		= dot(normalize(LightDirection.xyz), data.World_Normal);
	const float NdotL_Clamped		= max(NdotL, 0.0);
	const float NdotV_Clamped		= max(NdotV, 0.0);
	if (NdotL <= 0.f && NdotV <= 0.f) discard; // Discard if light will be zero anyway
	
	// Attenuation	
	const float Distance 			= length(LightDirection);
	const float	RadiusSquared		= lightBuffers[LightIndex].LightRadius * lightBuffers[LightIndex].LightRadius;
	const float range 				= 1.0f / RadiusSquared;
	const float Attenuation 		= 1.0f - (Distance * Distance) * (range * range);
	if (Attenuation <= 0.0f) 		discard; // Discard if outside of radius
	
	// Shadow
	const float cosAngle			= saturate(1.0f - NdotL);
	const float bias 				= clamp(0.005 * tan(acos(NdotL)), 0.0, 0.01);
	const vec3 scaledNormalOffset	= data.World_Normal * (cosAngle * ShadowSize_Recip);
	const float ShadowFactor 		= CalcShadowFactor(-(LightDirection + scaledNormalOffset), ViewDistance, RadiusSquared, bias);
	if (ShadowFactor <= EPSILON) 	discard; // Discard if completely in shadow
	
	// Direct Light	
	vec3 Fs;
	const vec3 Radiance 			= ( (lightBuffers[LightIndex].LightColor.xyz * lightBuffers[LightIndex].LightIntensity) * Attenuation ) * ShadowFactor;
	const vec3 D_Diffuse			= CalculateDiffuse( data.Albedo );
	const vec3 D_Specular			= BRDF_Specular( data.Roughness, data.Albedo, data.Metalness, data.World_Normal, LightDirection, NdotL_Clamped, NdotV_Clamped, ViewDirection, Fs);
	const vec3 D_Ratio				= (vec3(1.0f) - Fs) * (1.0f - data.Metalness); 
	LightingColor 					= (D_Ratio * D_Diffuse + D_Specular) * Radiance * NdotL_Clamped; 
}
