#version 460
#extension GL_ARB_bindless_texture : require
#extension GL_ARB_gpu_shader5 : require 
#extension GL_ARB_gpu_shader_int64 : require
#package "camera"

struct Geometry_Struct {
	uint materialID;
	mat4 mMatrix;
	mat4 bBoxMatrix;
};

layout (std430, binding = 0) readonly buffer Material_Buffer
{		
	uint64_t MaterialMaps[];
};

layout (std430, binding = 3) readonly buffer Visibility_Buffer {
	uint indexes[];
};

layout (std430, binding = 5) readonly buffer Geometry_Buffer {
	Geometry_Struct buffers[];
};

layout (location = 0) in vec3 vertex;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec3 tangent;
layout (location = 3) in vec3 bitangent;
layout (location = 4) in vec2 textureCoordinate;

layout (location = 0) out vec2 TexCoord;
layout (location = 1) out mat3 ViewTBN;
layout (location = 5) flat out sampler2DArray MaterialMap;

void main()
{	
	TexCoord             		= textureCoordinate;
	const mat3 vmMatrix			= mat3(vMatrix * buffers[indexes[gl_DrawID]].mMatrix);
	const vec3 ViewNormal 		= normalize(vmMatrix * normalize(normal));
	const vec3 ViewTangent		= normalize(vmMatrix * normalize(tangent));		
	const vec3 ViewBitangent 	= normalize(vmMatrix * normalize(bitangent));
	ViewTBN						= mat3(ViewTangent, ViewBitangent, ViewNormal);		
	MaterialMap 				= sampler2DArray(MaterialMaps[buffers[indexes[gl_DrawID]].materialID]);
	gl_Position           		= pMatrix * vMatrix * buffers[indexes[gl_DrawID]].mMatrix * vec4(vertex,1.0);			
}