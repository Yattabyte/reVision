/* Board Shader. */
#version 460
#package "Game\GameBuffer"

layout (std430, binding = 2) readonly coherent buffer Camera_Buffer {		
	mat4 pMatrix;
	mat4 vMatrix;
	mat4 pMatrix_Inverse;
	mat4 vMatrix_Inverse;
	vec3 EyePosition;
	vec2 CameraDimensions;
};

layout (location = 0) in vec3 vertex;
layout (location = 0) out vec2 TexCoord;

void main()
{	
	TexCoord = (vertex.xy + vec2(1.0)) / 2.0;
	gl_Position = pMatrix * vMatrix * boardMat * vec4(vertex.xy, -10, 1);
}