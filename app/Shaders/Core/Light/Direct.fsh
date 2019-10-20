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

float CalcDirectionalShadow(vec4 WorldSpacePos, vec3 World_Normal, float View_Depth, float NdotL)                                                  
{     
	const vec2 sampleOffsetDirections[9] = vec2[] (
		vec2(-1,  -1), vec2(-1,  0), vec2(-1,  1), 
		vec2(0,  -1), vec2(0,  0), vec2(0,  1),
		vec2(1,  -1), vec2(1,  0), vec2(1,  1)
	);
	const float cosAngle					= saturate(1.0f - NdotL);
	const float bias 						= clamp(0.005f * tan(acos(NdotL)), 0.0f, 0.005f);
	const vec4 scaledNormalOffset			= vec4(World_Normal * (cosAngle * ShadowSize_Recip), 0);
	int index = 0;
	for (; index < 4; ++index)
		if (View_Depth <= lightBuffers[lightIndex].CascadeEndClipSpace[index])
			break;	

	// Bring fragment coordinates from world space into light space, then into texture spaces
	const vec4 LightSpacePos				= lightBuffers[lightIndex].LightVP[index] * (WorldSpacePos + scaledNormalOffset);
	const vec3 ProjCoords 					= LightSpacePos.xyz / LightSpacePos.w;                                  
	const vec2 UVCoords 					= (0.5f * ProjCoords.xy + 0.5f);                                    
	const float FragmentDepth 				= (0.5f * ProjCoords.z + 0.5f) - EPSILON - bias; 		
	const float FactorAmt					= 1.0f / 9.0f;
	float Factor = 0.0f, depth = 0.0f;
	vec3 FinalCoord;
	for (uint x = 0; x < 9; ++x) { 
		FinalCoord 							= vec3( UVCoords + sampleOffsetDirections[x] * ShadowSize_Recip, lightBuffers[lightIndex].Shadow_Spot + index );
		depth 								= texture( ShadowMap, FinalCoord ).r;
		Factor 			   		  			+= (depth >= FragmentDepth) ? FactorAmt : 0.0f;	
	}
	return 									Factor;
} 

float CalcPointShadow(vec4 World_Pos, vec3 World_Normal, vec3 LightDirection, float ViewDistance, float NdotL, float LightRadius2)                                                  
{		
	const vec3 sampleOffsetDirections[20] = vec3[] (
		vec3( 1,  1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1,  1,  1), 
		vec3( 1,  1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1,  1, -1),
		vec3( 1,  1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1,  1,  0),
		vec3( 1,  0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1,  0, -1),
		vec3( 0,  1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0,  1, -1)
	);
	const float cosAngle					= saturate(1.0f - NdotL);
	const float bias 						= clamp(0.05f * tan(acos(NdotL)), 0.0f, 0.05f);
	const vec4 scaledNormalOffset			= vec4(World_Normal * (cosAngle * ShadowSize_Recip), 0);
	
	// Bring fragment coordinates from world space into light space, then into texture spaces
	float fragDepths[6];
	for (int index = 0; index < 6; ++index) {
		const vec4 lightSpacePos			= lightBuffers[lightIndex].LightVP[index] * (World_Pos + scaledNormalOffset);
		const vec3 ProjCoords				= lightSpacePos.xyz / lightSpacePos.w;    
		fragDepths[index]					= (0.5f * ProjCoords.z + 0.5f) - EPSILON;
	}
	const float diskRadius 					= (1.0f + (ViewDistance / LightRadius2)) * (ShadowSize_Recip * 2.0f);		
	const float FactorAmt					= 1.0f / 20.0f;
	float Factor = 0.0f, depth;
	vec3 FinalCoord;
	int faceIndex;
	for (uint x = 0; x < 20; ++x) {	
		FinalCoord							= vec3(LightDirection + sampleOffsetDirections[x] * diskRadius);			
		depth 								= texture(ShadowMap, sampleCube(FinalCoord, faceIndex) + vec3(0,0,lightBuffers[lightIndex].Shadow_Spot)).r;
		Factor 			   	 				+= (depth >= fragDepths[faceIndex] ) ? FactorAmt : 0.0;	
	}
	return 									Factor;
}

float CalcSpotShadow(vec4 World_Pos, vec3 World_Normal, float ViewDistance, float NdotL, float LightRadius2)                                                  
{		
	const vec2 sampleOffsetDirections[9] = vec2[] (
		vec2(-1,  -1), vec2(-1,  0), vec2(-1,  1), 
		vec2(0,  -1), vec2(0,  0), vec2(0,  1),
		vec2(1,  -1), vec2(1,  0), vec2(1,  1)
	);
	const float cosAngle					= saturate(1.0f - NdotL);
	const float bias 						= clamp(0.05f * tan(acos(NdotL)), 0.0f, 0.05f);
	const vec4 scaledNormalOffset			= vec4(World_Normal * (cosAngle * ShadowSize_Recip), 0);
	
	// Bring fragment coordinates from world space into light space, then into texture spaces
	const vec4 LightSpacePos				= lightBuffers[lightIndex].LightVP[0] * (World_Pos + scaledNormalOffset);
	const vec3 ProjCoords 					= LightSpacePos.xyz / LightSpacePos.w;                                  
	const vec2 UVCoords 					= (0.5f * ProjCoords.xy + 0.5f);                                    
	const float FragmentDepth 				= (0.5f * ProjCoords.z + 0.5f) - EPSILON - bias; 		
	const float diskRadius 					= (1.0f + (ViewDistance / LightRadius2)) * (ShadowSize_Recip * 2.0f);
	const float FactorAmt					= 1.0f / 9.0f;
	float Factor = 0.0f, depth = 0.0f;
	vec3 FinalCoord;
	for (uint x = 0; x < 9; ++x) { 
		FinalCoord 							= vec3(UVCoords + sampleOffsetDirections[x] * diskRadius, lightBuffers[lightIndex].Shadow_Spot);
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
	vec3 LightDirection 					= lightBuffers[lightIndex].LightPosition.xyz - data.World_Pos.xyz;
	const float LightLength					= length(LightDirection);
	LightDirection							= normalize(LightDirection);
	float SpotFactor						= clamp(1.0f - (1.0f - dot(LightDirection, lightBuffers[lightIndex].LightDirection.xyz)) * 1.0f / (1.0f - lightBuffers[lightIndex].LightCutoff), 0.0f, 1.0f);
	if (lightType == 0) {
		LightDirection						= lightBuffers[lightIndex].LightDirection.xyz;	
		SpotFactor							= 1.0f;
	}
	const vec3 DeltaView 					= EyePosition - data.World_Pos.xyz;  
	const float ViewDistance 				= length(DeltaView);
	const vec3 ViewDirection				= normalize(DeltaView);
	const float NdotV 						= dot(data.World_Normal, ViewDirection);
	const float NdotL 		 				= dot(LightDirection.xyz, data.World_Normal);
	const float NdotL_Clamped				= max(NdotL, 0.0f);
	const float NdotV_Clamped				= max(NdotV, 0.0f);
	
	// Shadow
	float ShadowFactor						= 1.0f;
	if (lightBuffers[lightIndex].Shadow_Spot >= 0) {
		if (lightType == 0)				
			ShadowFactor 					= CalcDirectionalShadow(data.World_Pos, data.World_Normal, -data.View_Pos.z, NdotL);		
		else if (lightType == 1) 
			ShadowFactor 					= CalcPointShadow(data.World_Pos, data.World_Normal.xyz, -LightDirection, ViewDistance, NdotL, lightBuffers[lightIndex].LightRadius * lightBuffers[lightIndex].LightRadius);		
		else if (lightType == 2)
			ShadowFactor 					= CalcSpotShadow(data.World_Pos, data.World_Normal.xyz, ViewDistance, NdotL, lightBuffers[lightIndex].LightRadius * lightBuffers[lightIndex].LightRadius);				
	}	
	
	// Attenuation	
	const float range 						= 1.0f / (lightBuffers[lightIndex].LightRadius * lightBuffers[lightIndex].LightRadius);
	const float Attenuation 				= clamp(1.0f - (LightLength * LightLength) * (range * range), 0.0f, 1.0f);	
	
	// Direct Light	
	vec3 Fs;
	const vec3 Radiance 					= lightBuffers[lightIndex].LightColor.xyz * lightBuffers[lightIndex].LightIntensity * Attenuation * SpotFactor * ShadowFactor;
	const vec3 D_Diffuse					= CalculateDiffuse( data.Albedo );
	const vec3 D_Specular					= BRDF_Specular( data.Roughness, data.Albedo, data.Metalness, data.World_Normal, LightDirection.xyz, NdotL_Clamped, NdotV_Clamped, ViewDirection, Fs);
	const vec3 D_Ratio						= (vec3(1.0f) - Fs) * (1.0f - data.Metalness);
	LightingColor		 					= (D_Ratio * D_Diffuse + D_Specular) * Radiance * NdotL_Clamped;
}