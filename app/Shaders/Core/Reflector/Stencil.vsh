/* Reflector - light stenciling shader. */
#version 460
#extension GL_ARB_shader_viewport_layer_array : enable
#package "CameraBuffer"

layout (location = 0) in vec3 vertex;

struct Reflection_Struct {
	mat4 mMatrix;
	mat4 rotMatrix;
	vec4 BoxCamPos;
	vec4 BoxScale;
	int CubeSpot;
};
layout (std430, binding = 4) readonly buffer Reflection_Index_Buffer {
	uint reflectionIndexes[];
};
layout (std430, binding = 8) readonly buffer Reflection_Buffer {
	Reflection_Struct reflectorBuffers[];
};


void main(void)
{	
	const int CamIndex = camIndexes[gl_InstanceID].x;
	gl_Position = camBuffer[CamIndex].pMatrix * camBuffer[CamIndex].vMatrix * reflectorBuffers[reflectionIndexes[gl_InstanceID]].mMatrix * vec4(vertex, 1);	
	gl_Layer = camIndexes[gl_InstanceID].y;
}

