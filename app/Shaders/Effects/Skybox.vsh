/* Calculates a 2D skybox from a quad. */
#version 460

layout (std430, binding = 2) readonly coherent buffer Camera_Buffer {		
	mat4 pMatrix;
	mat4 vMatrix;
	mat4 pMatrix_Inverse;
	mat4 vMatrix_Inverse;
	vec3 EyePosition;
	vec2 CameraDimensions;
};

layout (location = 0) in vec3 vertex;

layout (location = 0) out vec3 vecView;

void main()
{	
	vec4 posClip = vec4(vertex.xy, 1, 1);
	mat4 InvRotVMatrix = vMatrix_Inverse;
	InvRotVMatrix[3][0] = 0;
	InvRotVMatrix[3][1] = 0;
	InvRotVMatrix[3][2] = 0;
	vec4 vv =  InvRotVMatrix * pMatrix_Inverse * posClip;
	vecView = (vv.xyz / vv.w);
	vecView.y = -vecView.y;
	gl_Position = vec4(vertex.xy, 1, 1);	
}