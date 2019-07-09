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

layout (location = 0) flat out mat4 pMatrixInverse;
layout (location = 4) flat out mat4 vMatrixInverse;
layout (location = 8) flat out vec2 CameraDimensions;
layout (location = 9) flat out mat4 boxMatrix;
layout (location = 13) flat out mat4 rotMatrix;
layout (location = 17) flat out vec4 BoxCamPos;
layout (location = 18) flat out vec4 BoxScale;
layout (location = 19) flat out int CubeSpot;



void main(void)
{	
	const int CamIndex = camIndexes[gl_InstanceID].x;
	pMatrixInverse = camBuffer[CamIndex].pMatrixInverse;
	vMatrixInverse = camBuffer[CamIndex].vMatrixInverse;
	CameraDimensions = camBuffer[CamIndex].CameraDimensions;
	const uint ReflectorIndex = reflectionIndexes[gl_InstanceID];
	boxMatrix = reflectorBuffers[ReflectorIndex].mMatrix;
	rotMatrix = reflectorBuffers[ReflectorIndex].rotMatrix;
	BoxCamPos = reflectorBuffers[ReflectorIndex].BoxCamPos;
	BoxScale = reflectorBuffers[ReflectorIndex].BoxScale;
	CubeSpot = reflectorBuffers[ReflectorIndex].CubeSpot / 6;
	gl_Position = camBuffer[CamIndex].pvMatrix * reflectorBuffers[ReflectorIndex].mMatrix * vec4(vertex, 1);	
	gl_Layer = camIndexes[gl_InstanceID].y;
}