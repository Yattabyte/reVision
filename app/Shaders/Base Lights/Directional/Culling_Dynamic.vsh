#version 460
#extension GL_ARB_shader_viewport_layer_array : require
#define MAX_BONES 100 
#define NUM_CASCADES 4

struct Geometry_Struct {
	bool useBones;
	uint materialID;
	mat4 mMatrix;
	mat4 bBoxMatrix;
	mat4 Bones[MAX_BONES];
};

struct Light_Struct {
	mat4 lightV;
	vec4 LightColor;
	vec4 LightDirection;
	float ShadowSize_Recip;
	float LightIntensity;
	
	int Shadow_Spot;
	float CascadeEndClipSpace[NUM_CASCADES];
	mat4 LightVP[NUM_CASCADES];
	mat4 InverseLightVP[NUM_CASCADES];	
};

layout (std430, binding = 3) readonly buffer Visibility_Buffer {
	uint indexes[];
};
layout (std430, binding = 4) readonly buffer Geometry_Buffer {
	Geometry_Struct buffers[];
};
layout (std430, binding = 6) readonly buffer Light_Buffer_Reduced {
	Light_Struct lightBuffers[];
};

layout (location = 0) in vec3 vertex;
layout (location = 0) flat out int id;

layout (location = 0) uniform int LightIndex = 0;
layout (location = 1) uniform int Cascade = 0;

void main()
{	
	gl_Position = lightBuffers[LightIndex].LightVP[Cascade] * buffers[indexes[gl_DrawID]].bBoxMatrix * vec4(vertex,1.0);		
	gl_Layer = lightBuffers[LightIndex].Shadow_Spot + Cascade;
	id = gl_DrawID;
}

