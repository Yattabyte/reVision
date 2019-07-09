#version 460
#extension GL_ARB_shader_viewport_layer_array : enable
#package "CameraBuffer"

layout (location = 0) in vec3 vertex;

layout (location = 0) out vec2 TexCoord;
layout (location = 1) flat out mat4 CamPInverse;
layout (location = 5) flat out mat4 CamVInverse;


void main()
{
	const int CamIndex = camIndexes[gl_InstanceID].x;
	TexCoord = (vertex.xy + vec2(1.0)) / 2.0;	
	CamPInverse = inverse(camBuffer[CamIndex].pMatrix);
	CamVInverse = inverse(camBuffer[CamIndex].vMatrix);
	gl_Position = vec4(vertex, 1);
	gl_Layer = camIndexes[gl_InstanceID].y;
}
