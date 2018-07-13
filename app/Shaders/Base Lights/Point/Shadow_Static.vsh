#version 460
#extension GL_ARB_shader_viewport_layer_array : require
#extension GL_ARB_bindless_texture : require
#extension GL_ARB_gpu_shader5 : require 
#extension GL_ARB_gpu_shader_int64 : require

struct Geometry_Struct {
	uint materialID;
	mat4 mMatrix;
	mat4 bBoxMatrix;
};

struct Light_Struct {
	mat4 mMatrix;
	mat4 lightV; 
	mat4 lightPV[6];
	mat4 inversePV[6];
	vec4 LightColor;
	vec4 LightPosition;
	float ShadowSize_Recip;
	float LightIntensity;
	float LightRadius;
	int Shadow_Spot;
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

layout (std430, binding = 6) readonly buffer Light_Buffer {
	Light_Struct lightBuffers[];
};

layout (location = 0) in vec3 vertex;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec3 tangent;
layout (location = 3) in vec3 bitangent;
layout (location = 4) in vec2 textureCoordinate;

layout (location = 0) uniform int LightIndex = 0;

layout (location = 0) out vec3 WorldPos;
layout (location = 1) out vec3 LightPos;
layout (location = 2) out float FarPlane;
layout (location = 3) out mat3 WorldTBN;
layout (location = 7) out vec2 TexCoord0;
layout (location = 8) flat out vec3 ColorModifier;
layout (location = 9) flat out sampler2DArray MaterialMap;

void main()
{	
	const mat4 matTrans4 		= buffers[indexes[gl_DrawID]].mMatrix;
	const mat3 matTrans3 		= mat3(matTrans4);
	const vec4 WorldVertex		= matTrans4 * vec4(vertex,1.0);
	const vec3 WorldNormal 		= normalize(matTrans3 * normalize(normal));
	const vec3 WorldTangent		= normalize(matTrans3 * normalize(tangent));		
	const vec3 WorldBitangent 	= normalize(matTrans3 * normalize(bitangent));
	WorldPos					= WorldVertex.xyz / WorldVertex.w;
	FarPlane					= lightBuffers[LightIndex].LightRadius * lightBuffers[LightIndex].LightRadius;
	LightPos					= lightBuffers[LightIndex].LightPosition.xyz;
	WorldTBN					= mat3(WorldTangent, WorldBitangent, WorldNormal);	
	TexCoord0             		= textureCoordinate;		
	ColorModifier 				= lightBuffers[LightIndex].LightColor.xyz * lightBuffers[LightIndex].LightIntensity;
	MaterialMap 				= sampler2DArray(MaterialMaps[buffers[indexes[gl_DrawID]].materialID]);
	gl_Position           		= lightBuffers[LightIndex].lightPV[gl_InstanceID] * WorldVertex;	
	gl_Layer 					= lightBuffers[LightIndex].Shadow_Spot + gl_InstanceID;	
}