/* Screen space reflection shader - part 2 - Screen lookup from UV's */
#version 460
#extension GL_ARB_shader_viewport_layer_array : enable
#package "CameraBuffer"

layout (location = 0) in vec3 vertex;

layout (location = 0) out vec2 TexCoord;
layout (location = 1) flat out mat4 pMatrixInverse;
layout (location = 5) flat out mat4 vMatrixInverse;
layout (location = 9) flat out vec2 CameraDimensions;
layout (location = 10) flat out vec3 EyePosition;


void main(void)
{	
	const int CamIndex = camIndexes[gl_InstanceID].x;
	pMatrixInverse = camBuffer[CamIndex].pMatrixInverse;
	vMatrixInverse = camBuffer[CamIndex].vMatrixInverse;
	CameraDimensions = camBuffer[CamIndex].CameraDimensions;
	EyePosition = camBuffer[CamIndex].EyePosition;
	TexCoord = (vertex.xy + vec2(1.0)) / 2.0;
	gl_Position = vec4(vertex.xyz, 1);
	gl_Layer = camIndexes[gl_InstanceID].y;
}