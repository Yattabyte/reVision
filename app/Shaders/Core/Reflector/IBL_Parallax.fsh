#version 460
#pragma optionNV(fastmath on)
#pragma optionNV(ifcvt none)
#pragma optionNV(inline all)
#pragma optionNV(strict on)
#pragma optionNV(unroll all)
layout (early_fragment_tests) in;
#package "lighting_pbr"

struct Reflection_Struct {
	mat4 mMatrix;
	mat4 rotMatrix;
	vec4 BoxCamPos;
	vec4 BoxScale;
	int CubeSpot;
};

layout (std430, binding = 8) readonly buffer Reflection_Buffer {
	Reflection_Struct reflectorBuffers[];
};

layout (binding = 4) uniform samplerCubeArray ReflectionMap;

layout (location = 0) flat in uint ReflectorIndex;

layout (location = 0) out vec3 ReflectionColor;

vec2 CalcTexCoord()
{
    return			 				gl_FragCoord.xy / cameraBuffer.CameraDimensions;
}

vec3 ParallaxCorrectCubemap(in vec3 ReflectDir, in mat4 bboxMat, in vec3 bboxPos, in vec3 WorldPos)
{   
	mat4 InvBBoxMat					= inverse(bboxMat);
	vec3 RayLS						= mat3(InvBBoxMat) * ReflectDir;
    vec3 PositionLS 				= (InvBBoxMat * vec4(WorldPos, 1)).xyz;
	
	vec3 MaxPlaneIntersection 		= (vec3(1) - PositionLS) / RayLS;
    vec3 MinPlaneIntersection 		= (vec3(-1) - PositionLS) / RayLS;
   
    vec3 LargestIntersection		= max(MaxPlaneIntersection, MinPlaneIntersection);	
	float ClosestPoint				= min(min(LargestIntersection.x, LargestIntersection.y), LargestIntersection.z);
	
    vec3 IntersectionPos 			= WorldPos.xyz + ReflectDir * ClosestPoint;
	
    return							(IntersectionPos - bboxPos);
}

vec3 CalculateReflections(in vec3 WorldPos, in vec3 ViewPos, in vec3 ViewNormal, in float Roughness)
{		
	vec3 ReflectDir					= reflect(ViewPos, ViewNormal);
		 ReflectDir 				= normalize(cameraBuffer.vMatrix_Inverse * vec4(ReflectDir, 0)).xyz;
	vec3 CorrectedDir				= ParallaxCorrectCubemap
									(	ReflectDir, 
										reflectorBuffers[ReflectorIndex].mMatrix,
										reflectorBuffers[ReflectorIndex].BoxCamPos.xyz, 
										WorldPos.xyz	);	
	//return							texture(ReflectionMap, vec4(CorrectedDir, Roughness * 5.0f)).xyz;	
	return							textureLod(ReflectionMap, vec4(CorrectedDir, reflectorBuffers[ReflectorIndex].CubeSpot), Roughness * 5.0f).xyz;		
}

void main(void)
{   
	ViewData data;
	GetFragmentData(CalcTexCoord(), data);
	
	vec4 rotWorldPos					= reflectorBuffers[ReflectorIndex].rotMatrix * vec4(data.World_Pos.xyz, 1.0f);
	rotWorldPos.xyz /= rotWorldPos.w;
	const vec3 BBoxMin 					= reflectorBuffers[ReflectorIndex].BoxCamPos.xyz - reflectorBuffers[ReflectorIndex].BoxScale.xyz;
	const vec3 BBoxMax 					= reflectorBuffers[ReflectorIndex].BoxCamPos.xyz + reflectorBuffers[ReflectorIndex].BoxScale.xyz;
	if (rotWorldPos.x < BBoxMin.x || rotWorldPos.y < BBoxMin.y || rotWorldPos.z < BBoxMin.z
	|| rotWorldPos.x > BBoxMax.x || rotWorldPos.y > BBoxMax.y || rotWorldPos.z > BBoxMax.z)
		discard;
	
	ReflectionColor						= CalculateReflections(data.World_Pos.xyz, data.View_Pos.xyz, data.View_Normal, data.Roughness);
}