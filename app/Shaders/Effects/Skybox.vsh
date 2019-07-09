/* Calculates a 2D skybox from a quad. */
#version 460
#extension GL_ARB_shader_viewport_layer_array : enable
#package "CameraBuffer"

layout (location = 0) in vec3 vertex;

layout (location = 0) out vec3 vecView;


void main()
{	
	const int CamIndex = camIndexes[gl_InstanceID].x;
	vec4 posClip = vec4(vertex.xy, 1, 1);
	mat4 InvRotVMatrix = camBuffer[CamIndex].vMatrixInverse;
	InvRotVMatrix[3][0] = 0;
	InvRotVMatrix[3][1] = 0;
	InvRotVMatrix[3][2] = 0;
	vec4 vv =  InvRotVMatrix * camBuffer[CamIndex].pMatrixInverse * posClip;
	vecView = (vv.xyz / vv.w);
	vecView.y = -vecView.y;
	gl_Position = vec4(vertex.xy, 1, 1);	
	gl_Layer = camIndexes[gl_InstanceID].y;
}