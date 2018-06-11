#version 460
#extension GL_ARB_bindless_texture : require
#extension GL_ARB_gpu_shader5 : require 
#extension GL_ARB_gpu_shader_int64 : require
#package "camera"
#define MAX_BONES 100 

struct Geometry_Struct {
	bool useBones;
	uint materialID;
	mat4 mMatrix;
	mat4 bBoxMatrix;
	mat4 Bones[MAX_BONES];
};

layout (std430, binding = 0) readonly buffer Material_Buffer
{		
	uint64_t MaterialMaps[];
};

layout (std430, binding = 3) readonly buffer Visibility_Buffer {
	uint indexes[];
};

layout (std430, binding = 4) readonly buffer Geometry_Buffer {
	Geometry_Struct buffers[];
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
	mat4 BoneTransform 			= mat4(1.0);
	if (buffers[indexes[gl_DrawID]].useBones) {	
		BoneTransform 			= buffers[indexes[gl_DrawID]].Bones[BoneIDs[0]] * Weights[0];
		BoneTransform 	   	   += buffers[indexes[gl_DrawID]].Bones[BoneIDs[1]] * Weights[1];
		BoneTransform          += buffers[indexes[gl_DrawID]].Bones[BoneIDs[2]] * Weights[2];
		BoneTransform          += buffers[indexes[gl_DrawID]].Bones[BoneIDs[3]] * Weights[3];
	}
	TexCoord             		= textureCoordinate;
	const mat3 vmMatrix			= mat3(vMatrix * buffers[indexes[gl_DrawID]].mMatrix * BoneTransform);
	const vec3 ViewNormal 		= normalize(vmMatrix * normalize(normal));
	const vec3 ViewTangent		= normalize(vmMatrix * normalize(tangent));		
	const vec3 ViewBitangent 	= normalize(vmMatrix * normalize(bitangent));
	ViewTBN						= mat3(ViewTangent, ViewBitangent, ViewNormal);		
	MaterialMap 				= sampler2DArray(MaterialMaps[buffers[indexes[gl_DrawID]].materialID]);
	gl_Position           		= pMatrix * vMatrix * buffers[indexes[gl_DrawID]].mMatrix * BoneTransform * vec4(vertex,1.0);			
}

