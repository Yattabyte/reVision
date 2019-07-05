/* Prop - Geometry shadowing shader. */
#version 460
#extension GL_ARB_shader_viewport_layer_array : enable
#define MAX_BONES 100
#define TEXTURES_PER_MATERIAL 3

struct CamAttributes {
	float farPlane;
	int layer;
	vec3 eyePosition;
	mat4 pvMatrix;		
};
struct PropAttributes {
	uint materialID;
	uint isStatic;
	mat4 mMatrix;
	mat4 bBoxMatrix;
};
struct BonesStruct {
	mat4 bones[MAX_BONES];
};
layout (std430, binding = 2) readonly buffer Cam_Buffer {
	CamAttributes camBuffer[];
};
layout (std430, binding = 3) readonly buffer Prop_Buffer {
	PropAttributes propBuffer[];
};
layout (std430, binding = 4) readonly buffer Cam_Index_Buffer {
	uint camIndexes[];
};
layout (std430, binding = 5) readonly buffer Prop_Index_Buffer {
	uint propIndexes[];
};
layout (std430, binding = 6) readonly buffer Skeleton_Buffer {
	BonesStruct skeletonBuffer[];
};
layout (std430, binding = 7) readonly buffer Skeleton_Index_Buffer {
	int skeletonIndexes[];
};

layout (location = 0) in vec3 vertex;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec3 tangent;
layout (location = 3) in vec3 bitangent;
layout (location = 4) in vec2 textureCoordinate;
layout (location = 5) in uint matID;
layout (location = 6) in ivec4 boneIDs;
layout (location = 7) in vec4 weights;

layout (location = 0) uniform int layer = 0;

layout (location = 0) out vec3 WorldPos;
layout (location = 1) out mat3 WorldTBN;
layout (location = 5) out vec2 TexCoord0;
layout (location = 6) flat out uint MaterialOffset;
layout (location = 7) flat out vec3 EyePosition;
layout (location = 8) flat out float FarPlane;


void main()
{	
	const uint CamIndex 		= camIndexes[gl_DrawID];
	const uint PropIndex 		= propIndexes[gl_DrawID];
	const int SkeletonIndex 	= skeletonIndexes[gl_DrawID];
	mat4 BoneTransform 			= mat4(1.0);
	if (SkeletonIndex >= 0) {	
		BoneTransform 			= skeletonBuffer[SkeletonIndex].bones[boneIDs[0]] * weights[0];
		BoneTransform 	   	   += skeletonBuffer[SkeletonIndex].bones[boneIDs[1]] * weights[1];
		BoneTransform          += skeletonBuffer[SkeletonIndex].bones[boneIDs[2]] * weights[2];
		BoneTransform          += skeletonBuffer[SkeletonIndex].bones[boneIDs[3]] * weights[3];
	}
	const mat4 matTrans4 		= propBuffer[PropIndex].mMatrix * BoneTransform;
	const mat3 matTrans3 		= mat3(matTrans4);
	const vec4 WorldVertex		= matTrans4 * vec4(vertex,1.0);
	const vec3 WorldNormal 		= normalize(matTrans3 * normalize(normal));
	const vec3 WorldTangent		= normalize(matTrans3 * normalize(tangent));		
	const vec3 WorldBitangent 	= normalize(matTrans3 * normalize(bitangent));
	WorldPos					= WorldVertex.xyz / WorldVertex.w;
	WorldTBN					= mat3(WorldTangent, WorldBitangent, WorldNormal);
	TexCoord0             		= textureCoordinate;	
	MaterialOffset				= matID + (propBuffer[PropIndex].materialID * TEXTURES_PER_MATERIAL);
	gl_Position           		= camBuffer[CamIndex].pvMatrix * WorldVertex;
	gl_Layer 					= camBuffer[CamIndex].layer;
	EyePosition					= camBuffer[CamIndex].eyePosition;
	FarPlane					= camBuffer[CamIndex].farPlane;
}