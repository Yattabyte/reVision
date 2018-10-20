/* Prop - Geometry rendering shader. */
#version 460
#define MAX_BONES 100
#define TEXTURES_PER_MATERIAL 3

struct PropAttributes {
	uint materialID;
	mat4 mMatrix;
	mat4 bBoxMatrix;
};
struct BonesStruct {
	mat4 bones[MAX_BONES];
};

layout (std430, binding = 2) readonly coherent buffer Camera_Buffer {		
	mat4 pMatrix;
	mat4 vMatrix;
	mat4 pMatrix_Inverse;
	mat4 vMatrix_Inverse;
	vec3 EyePosition;
	vec2 CameraDimensions;
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
layout (location = 5) in uint matID;
layout (location = 6) in ivec4 BoneIDs;
layout (location = 7) in vec4 Weights;

layout (location = 0) out vec2 TexCoord;
layout (location = 1) out mat3 ViewTBN;
layout (location = 5) flat out uint MaterialOffset;

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
	const mat4 vmMatrix4		= vMatrix * propBuffer[PropIndex].mMatrix * BoneTransform;
	const mat3 vmMatrix3		= mat3(vmMatrix4);
	const vec3 ViewNormal 		= normalize(vmMatrix3 * normalize(normal));
	const vec3 ViewTangent		= normalize(vmMatrix3 * normalize(tangent));		
	const vec3 ViewBitangent 	= normalize(vmMatrix3 * normalize(bitangent));
	ViewTBN						= mat3(ViewTangent, ViewBitangent, ViewNormal);		
	MaterialOffset				= matID + (propBuffer[PropIndex].materialID * TEXTURES_PER_MATERIAL);
	gl_Position           		= pMatrix * vmMatrix4 * vec4(vertex,1.0);			
}