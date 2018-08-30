#version 460
#extension GL_ARB_bindless_texture : require
#extension GL_ARB_gpu_shader5 : require 
#extension GL_ARB_gpu_shader_int64 : require
#extension GL_ARB_shader_viewport_layer_array : require
#define MAX_BONES 100

struct PropAttributes {
	uint materialID;
	mat4 mMatrix;
	mat4 bBoxMatrix;
};
struct BonesStruct {
	mat4 bones[MAX_BONES];
};
struct Reflection_Struct {
	mat4 mMatrix;
	mat4 rotMatrix;
	mat4 pMatrix;
	mat4 vMatrix[6];
	vec4 BoxCamPos;
	vec4 BoxScale;
	int CubeSpot;
};

layout (std430, binding = 0) readonly buffer Material_Buffer {		
	uint64_t MaterialMaps[];
};
layout (std430, binding = 3) readonly buffer Prop_Buffer {
	PropAttributes propBuffer[];
};
layout (std430, binding = 4) readonly buffer Prop_Index_Buffer {
	uint propIndexes[];
};
layout (std430, binding = 5) readonly buffer Skeleton_Buffer {
	BonesStruct skeletonBuffer[];
};
layout (std430, binding = 6) readonly buffer Skeleton_Index_Buffer {
	int skeletonIndexes[];
};
layout (std430, binding = 8) readonly buffer Reflection_Buffer {
	Reflection_Struct reflectorBuffers[];
};

// Uniform Inputs
layout (location = 0) uniform int reflectorIndex = 0;
layout (location = 1) uniform int instance = 0;

layout (location = 0) in vec3 vertex;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec3 tangent;
layout (location = 3) in vec3 bitangent;
layout (location = 4) in vec2 textureCoordinate;
layout (location = 5) in ivec4 BoneIDs;
layout (location = 6) in vec4 Weights;

layout (location = 0) out vec2 TexCoord;
layout (location = 1) out mat3 ViewTBN;
layout (location = 5) flat out sampler2DArray MaterialMap;

void main()
{	
	const uint PropIndex 		= propIndexes[gl_DrawID];
	const int SkeletonIndex 	= skeletonIndexes[gl_DrawID];
	mat4 BoneTransform 			= mat4(1.0);
	if (SkeletonIndex != -1) {	
		BoneTransform 			= skeletonBuffer[SkeletonIndex].bones[BoneIDs[0]] * Weights[0];
		BoneTransform 	   	   += skeletonBuffer[SkeletonIndex].bones[BoneIDs[1]] * Weights[1];
		BoneTransform          += skeletonBuffer[SkeletonIndex].bones[BoneIDs[2]] * Weights[2];
		BoneTransform          += skeletonBuffer[SkeletonIndex].bones[BoneIDs[3]] * Weights[3];
	}
	TexCoord             		= textureCoordinate;
	const mat3 vmMatrix			= mat3(reflectorBuffers[reflectorIndex].vMatrix[instance] * propBuffer[PropIndex].mMatrix * BoneTransform);
	const vec3 ViewNormal 		= normalize(vmMatrix * normalize(normal));
	const vec3 ViewTangent		= normalize(vmMatrix * normalize(tangent));		
	const vec3 ViewBitangent 	= normalize(vmMatrix * normalize(bitangent));
	ViewTBN						= mat3(ViewTangent, ViewBitangent, ViewNormal);		
	MaterialMap 				= sampler2DArray(MaterialMaps[propBuffer[PropIndex].materialID]);
	gl_Position           		= reflectorBuffers[reflectorIndex].pMatrix * reflectorBuffers[reflectorIndex].vMatrix[instance] * propBuffer[PropIndex].mMatrix * BoneTransform * vec4(vertex,1.0);				
}