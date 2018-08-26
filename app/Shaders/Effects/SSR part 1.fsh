#version 460
#extension GL_ARB_bindless_texture : require
#pragma optionNV(fastmath on)
#pragma optionNV(fastprecision on)
#pragma optionNV(ifcvt none)
#pragma optionNV(inline all)
#pragma optionNV(strict on)
#pragma optionNV(unroll all)
#package "camera"

// SSR Variables
const float rayStep = 0.01f;
const uint maxSteps = 6u;
const uint maxBinarySteps = 4u;
const float maxDistance = 500.0f;

// The screen texture
layout (location = 0, bindless_sampler) uniform sampler2D BayerMatrix;
layout (binding = 0) uniform sampler2D ColorMap;
layout (binding = 1) uniform sampler2D ViewNormalMap;
layout (binding = 2) uniform sampler2D SpecularMap;
layout (binding = 3) uniform sampler2D DepthMap;

layout (location = 0) in vec2 TexCoord;
layout (location = 0) out vec4 LightingColor;

struct ViewData {
	vec4 World_Pos;
	vec4 View_Pos;
	vec3 World_Normal;
	vec3 View_Normal;
	float View_Depth;
};

void GetFragmentData(in vec2 TexCoord, out ViewData data)
{
	const vec4 Texture2				= texture(ViewNormalMap, TexCoord);
	const vec4 Texture4				= texture(DepthMap, TexCoord);
	
	data.View_Normal				= Texture2.rgb;
	data.View_Depth					= Texture4.r;
	
	data.View_Pos					= cameraBuffer.pMatrix_Inverse * vec4(vec3(TexCoord, data.View_Depth) * 2.0f - 1.0f, 1.0f);
    data.World_Pos 					= cameraBuffer.vMatrix_Inverse * data.View_Pos;
	data.View_Pos 					= data.View_Pos / data.View_Pos.w;
	data.World_Pos 					= data.World_Pos / data.World_Pos.w;
    data.World_Normal 				= normalize((cameraBuffer.vMatrix_Inverse * vec4(data.View_Normal, 0))).xyz;
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

void main(void)
{   
	ViewData data;
	GetFragmentData(TexCoord, data);
	// Quit Early
	if (data.View_Depth	>= 1.0f)
		discard;
	
	// Variables
	const vec3 SSPos	 				= vec3(TexCoord, data.View_Depth);
    const vec3 WorldNormal 				= normalize((cameraBuffer.vMatrix_Inverse * vec4(data.View_Normal, 0))).xyz;
	const vec3 CameraVector 			= normalize(data.World_Pos.xyz - cameraBuffer.EyePosition);
	const vec3 ReflectionVector 		= reflect(CameraVector, WorldNormal);
	const vec3 ViewPos_Unit				= normalize(data.View_Pos.xyz); 	
	const vec3 reflected 				= normalize(reflect(ViewPos_Unit, normalize(data.View_Normal))); 
	const float rDotV					= dot(reflected, ViewPos_Unit);
	const vec4 PointAlongReflectionVec 	= vec4(maxDistance * ReflectionVector + data.World_Pos.xyz, 1.0f);
	vec4 SSReflectionPoint				= cameraBuffer.pMatrix * cameraBuffer.vMatrix * PointAlongReflectionVec;
	SSReflectionPoint 					/= SSReflectionPoint.w;
	SSReflectionPoint.xy 				= SSReflectionPoint.xy * 0.5 + 0.5;
	const vec3 SSReflectionVector		= normalize(SSReflectionPoint.xyz - SSPos.xyz);
	
	// Ray March
	const vec3 ReflectionUVS 			= RayMarch(SSReflectionVector, SSPos.xyz);	
	if (ReflectionUVS.x <= 1.0f && ReflectionUVS.x >= 0.0f && ReflectionUVS.y <= 1.0f && ReflectionUVS.y >= 0.0f) 
		LightingColor 						= vec4(ReflectionUVS, rDotV);
}

