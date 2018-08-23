#version 460
#extension GL_ARB_shader_draw_parameters : enable
#package "camera"

layout (location = 0) in vec3 vertex;

struct Reflection_Struct {
	mat4 mMatrix;
	mat4 rotMatrix;
	vec4 BoxCamPos;
	vec4 BoxScale;
	int CubeSpot;
};

layout (std430, binding = 3) readonly buffer Reflection_Index_Buffer {
	uint reflectionIndexes[];
};
layout (std430, binding = 8) readonly buffer Reflection_Buffer {
	Reflection_Struct reflectorBuffers[];
};

layout (location = 0) flat out uint BufferIndex;

void main(void)
{	
	BufferIndex 		= gl_InstanceID;
	gl_Position 		= cameraBuffer.pMatrix * cameraBuffer.vMatrix * reflectorBuffers[reflectionIndexes[gl_InstanceID]].mMatrix * vec4(vertex, 1);	
}

