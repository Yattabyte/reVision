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

struct Light_Struct {
	mat4 shadowVP[6];
	mat4 mMatrix;
	vec4 LightColor;
	vec4 LightPosition;
	float LightIntensity;
	float LightRadius;
	int Shadow_Spot;
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

layout (location = 0) flat in mat4 CamPInverse;
layout (location = 4) flat in mat4 CamVInverse;
layout (location = 8) flat in vec3 CamEyePosition;
layout (location = 9) flat in vec2 CamDimensions;
layout (location = 10) flat in vec3 LightColorInt;
layout (location = 11) flat in vec3 LightPosition;
layout (location = 12) flat in float LightRadius2;
layout (location = 13) flat in int ShadowSpotFinal;
layout (location = 14) flat in int lightIndex;

layout (location = 0) out vec3 LightingColor; 

// Use PBR lighting methods
#package "lighting_pbr"


const vec3 sampleOffsetDirections[20] = vec3[] (
	vec3( 1,  1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1,  1,  1), 
	vec3( 1,  1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1,  1, -1),
	vec3( 1,  1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1,  1,  0),
	vec3( 1,  0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1,  0, -1),
	vec3( 0,  1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0,  1, -1)
);
	
vec2 CalcTexCoord()
{
    return			 				gl_FragCoord.xy / CamDimensions;
}

vec3 sampleCube(vec3 v, out int face)
{
	vec3 vAbs = abs(v);
	float ma;
	vec2 uv;
	float faceIndex;
	if(vAbs.z >= vAbs.x && vAbs.z >= vAbs.y) {
		faceIndex = v.z < 0.0 ? 5.0 : 4.0;
		ma = 0.5 / vAbs.z;
		uv = vec2(v.z < 0.0 ? -v.x : v.x, -v.y);
	}
	else if(vAbs.y >= vAbs.x) {
		faceIndex = v.y < 0.0 ? 3.0 : 2.0;
		ma = 0.5 / vAbs.y;
		uv = vec2(v.x, v.y < 0.0 ? -v.z : v.z);
	}
	else {
		faceIndex = v.x < 0.0 ? 1.0 : 0.0;
		ma = 0.5 / vAbs.x;
		uv = vec2(v.x < 0.0 ? v.z : -v.z, -v.y);
	}
	face = int(faceIndex);
	return vec3(uv * ma + 0.5, faceIndex);
}

float CalcShadowFactor(in vec3 LightDirection, in vec4 WorldPos, in float ViewDistance, in float bias)                                                  
{		
	if (ShadowSpotFinal >= 0) {
		const float FactorAmt = 1.0f / 20.0f;
		float fragDepths[6];
		for (int x = 0; x < 6; ++x) {
			const vec4 lightSpacePos = lightBuffers[lightIndex].shadowVP[x] * WorldPos;
			const vec3 ProjCoords = lightSpacePos.xyz / lightSpacePos.w;    
			fragDepths[x] = (0.5f * ProjCoords.z + 0.5f) - EPSILON;
		}
		const float diskRadius 		= (1.0f + (ViewDistance / LightRadius2)) * (ShadowSize_Recip * 2.0f);
		
		float Factor = 0.0f, depth;
		vec3 FinalCoord;
		int faceIndex;
		for (uint x = 0; x < 20; ++x) {	
			FinalCoord				= vec3(LightDirection + sampleOffsetDirections[x] * diskRadius);			
			depth 					= texture(ShadowMap, sampleCube(FinalCoord, faceIndex) + vec3(0,0,ShadowSpotFinal)).r;
			Factor 			   	 	+= (depth >= fragDepths[faceIndex] ) ? FactorAmt : 0.0;	
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
	const vec4 scaledNormalOffset	= vec4(data.World_Normal * (cosAngle * ShadowSize_Recip), 0);
	const float ShadowFactor 		= CalcShadowFactor(-LightDirection, data.World_Pos + scaledNormalOffset, ViewDistance, bias);
	if (ShadowFactor <= EPSILON) 	discard; // Discard if completely in shadow
	
	// Direct Light	
	vec3 Fs;
	const vec3 Radiance 			= (LightColorInt * Attenuation) * ShadowFactor;
	const vec3 D_Diffuse			= CalculateDiffuse( data.Albedo );
	const vec3 D_Specular			= BRDF_Specular( data.Roughness, data.Albedo, data.Metalness, data.World_Normal, LightDirection, NdotL_Clamped, NdotV_Clamped, ViewDirection, Fs);
	const vec3 D_Ratio				= (vec3(1.0f) - Fs) * (1.0f - data.Metalness); 
	LightingColor 					= (D_Ratio * D_Diffuse + D_Specular) * Radiance * NdotL_Clamped; 
}
