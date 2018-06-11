#version 460
#extension GL_ARB_shader_viewport_layer_array : require
#package "camera"
#define MAX_BONES 100 

struct Geometry_Struct {
	bool useBones;
	uint materialID;
	mat4 mMatrix;
	mat4 bBoxMatrix;
	mat4 Bones[MAX_BONES];
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

layout (std430, binding = 3) readonly buffer Visibility_Buffer {
	uint indexes[];
};
layout (std430, binding = 4) readonly buffer Geometry_Buffer {
	Geometry_Struct buffers[];
};
layout (std430, binding = 6) readonly buffer Light_Buffer {
	Light_Struct lightBuffers[];
};

layout (location = 0) in vec3 vertex;
layout (location = 0) flat out int id;

layout (location = 0) uniform int LightIndex = 0;

void main()
{	
	gl_Position = lightBuffers[LightIndex].lightPV[gl_InstanceID] * buffers[indexes[gl_DrawID]].bBoxMatrix * vec4(vertex, 1.0);		
	gl_Layer = lightBuffers[LightIndex].Shadow_Spot + gl_InstanceID;
	id = gl_DrawID;
}