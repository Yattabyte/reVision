#version 460
#extension GL_ARB_bindless_texture : require
#extension GL_ARB_gpu_shader5 : require 
#extension GL_ARB_gpu_shader_int64 : require
#define MAX_BONES 100
#package "camera"

struct PropAttributes {
	uint materialID;
	mat4 mMatrix;
	mat4 bBoxMatrix;
};
struct BonesStruct {
	mat4 bones[MAX_BONES];
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
	const mat4 vmMatrix4		= cameraBuffer.vMatrix * propBuffer[PropIndex].mMatrix * BoneTransform;
	const mat3 vmMatrix3		= mat3(vmMatrix4);
	const vec3 ViewNormal 		= normalize(vmMatrix3 * normalize(normal));
	const vec3 ViewTangent		= normalize(vmMatrix3 * normalize(tangent));		
	const vec3 ViewBitangent 	= normalize(vmMatrix3 * normalize(bitangent));
	ViewTBN						= mat3(ViewTangent, ViewBitangent, ViewNormal);		
	MaterialMap 				= sampler2DArray(MaterialMaps[propBuffer[PropIndex].materialID]);
	gl_Position           		= cameraBuffer.pMatrix * vmMatrix4 * vec4(vertex,1.0);			
}