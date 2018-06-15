#version 460
#extension GL_ARB_shader_draw_parameters : enable
#package "camera"

layout (location = 0) in vec3 vertex;

struct Reflection_Struct {
	mat4 mMatrix;
	vec4 BoxCamPos;
	float Radius;
	int CubeSpot;
};
layout (std430, binding = 3) readonly buffer Visibility_Buffer {
	uint indexes[];
};
layout (std430, binding = 5) readonly buffer Reflection_Buffer {
	Reflection_Struct buffers[];
};

layout (location = 0) flat out uint BufferIndex;

void main(void)
{	
	BufferIndex 		= gl_InstanceID;
	gl_Position 		= pMatrix * vMatrix * buffers[indexes[gl_InstanceID]].mMatrix * vec4(vertex, 1);	
}

