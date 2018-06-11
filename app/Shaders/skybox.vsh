#version 460
#package "camera"

layout(location = 0) in vec3 vertex;

out vec3 vecView;

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