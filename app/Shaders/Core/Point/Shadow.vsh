/* Point light - geometry shadowing shader. */
#version 460
#extension GL_ARB_shader_viewport_layer_array : require
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
layout (std430, binding = 8) readonly buffer Light_Buffer {
	Light_Struct lightBuffers[];
};
layout (std430, binding = 9) readonly buffer Shadow_Buffer {
	Shadow_Struct shadowBuffers[];
};

layout (location = 0) in vec3 vertex;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec3 tangent;
layout (location = 3) in vec3 bitangent;
layout (location = 4) in vec2 textureCoordinate;
layout (location = 5) in uint matID;
layout (location = 6) in ivec4 boneIDs;
layout (location = 7) in vec4 weights;

layout (location = 0) uniform int LightIndex = 0;
layout (location = 1) uniform int ShadowIndex = 0;

layout (location = 0) out vec3 WorldPos;
layout (location = 1) out vec3 LightPos;
layout (location = 2) out float FarPlane;
layout (location = 3) out mat3 WorldTBN;
layout (location = 7) out vec2 TexCoord0;
layout (location = 8) flat out vec3 ColorModifier;
layout (location = 9) flat out uint MaterialOffset;

void main()
{	
	const uint PropIndex 		= propIndexes[gl_DrawID];
	const int SkeletonIndex 	= skeletonIndexes[gl_DrawID];
	mat4 BoneTransform 			= mat4(1.0);
	const bool isStatic 		= SkeletonIndex == -1 ? true : false;
	if (!isStatic) {	
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
	LightPos					= lightBuffers[LightIndex].LightPosition.xyz;
	FarPlane					= lightBuffers[LightIndex].LightRadius * lightBuffers[LightIndex].LightRadius;
	WorldTBN					= mat3(WorldTangent, WorldBitangent, WorldNormal);	
	TexCoord0             		= textureCoordinate;		
	ColorModifier 				= lightBuffers[LightIndex].LightColor.xyz * lightBuffers[LightIndex].LightIntensity;	
	MaterialOffset				= matID + (propBuffer[PropIndex].materialID * TEXTURES_PER_MATERIAL);
	gl_Position           		= shadowBuffers[ShadowIndex].lightPV[gl_InstanceID] * WorldVertex;	
	gl_Layer 					= shadowBuffers[ShadowIndex].Shadow_Spot + gl_InstanceID + (int(isStatic) * 6);	
}