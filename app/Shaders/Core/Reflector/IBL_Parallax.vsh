/* Reflector - lighting shader. */
#version 460
#extension GL_ARB_shader_draw_parameters : enable
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

layout (location = 0) flat out mat4 mMatrix;
layout (location = 4) flat out mat4 rotMatrix;
layout (location = 8) flat out vec4 BoxCamPos;
layout (location = 9) flat out vec4 BoxScale;
layout (location = 10) flat out int CubeSpot;
layout (location = 11) flat out mat4 CamPInverse;
layout (location = 15) flat out mat4 CamVInverse;
layout (location = 19) flat out vec2 CamDimensions;


void main(void)
{	
	const int CamIndex = camIndexes[gl_InstanceID].x;
	const uint ReflectorIndex = reflectionIndexes[gl_InstanceID];
	mMatrix = reflectorBuffers[ReflectorIndex].mMatrix;
	rotMatrix = reflectorBuffers[ReflectorIndex].rotMatrix;
	BoxCamPos = reflectorBuffers[ReflectorIndex].BoxCamPos;
	BoxScale = reflectorBuffers[ReflectorIndex].BoxScale;
	CubeSpot = reflectorBuffers[ReflectorIndex].CubeSpot / 6;
	CamPInverse = inverse(camBuffer[CamIndex].pMatrix);
	CamVInverse = inverse(camBuffer[CamIndex].vMatrix);
	CamDimensions = camBuffer[CamIndex].CameraDimensions;
	gl_Position = camBuffer[CamIndex].pMatrix * camBuffer[CamIndex].vMatrix * reflectorBuffers[ReflectorIndex].mMatrix * vec4(vertex, 1);	
	gl_Layer = camIndexes[gl_InstanceID].y;
}