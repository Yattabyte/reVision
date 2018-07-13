#version 460
#extension GL_ARB_shader_viewport_layer_array : require
#package "camera"

struct Geometry_Struct {
	uint materialID;
	mat4 mMatrix;
	mat4 bBoxMatrix;
};

struct Light_Struct {
	mat4 lightM;
	mat4 lightV;
	mat4 lightPV;
	mat4 InverseLightPV;
	vec4 LightColor;
	vec4 LightPosition;
	vec4 LightVector;
	float ShadowSize_Recip;
	float LightIntensity;
	float LightRadius;
	float LightCutoff;
	int Shadow_Spot;
};

layout (std430, binding = 3) readonly buffer Visibility_Buffer {
	uint indexes[];
};
layout (std430, binding = 5) readonly buffer Geometry_Buffer {
	Geometry_Struct buffers[];
};
layout (std430, binding = 6) readonly buffer Light_Buffer_Reduced {
	Light_Struct lightBuffers[];
};
	
layout (location = 0) in vec3 vertex;
layout (location = 0) flat out int id;

layout (location = 0) uniform int LightIndex = 0;

void main()
{	
	gl_Position =  lightBuffers[LightIndex].lightPV * buffers[indexes[gl_DrawID]].bBoxMatrix * vec4(vertex,1.0);		
	gl_Layer = lightBuffers[LightIndex].Shadow_Spot;
	id = gl_DrawID;
}

