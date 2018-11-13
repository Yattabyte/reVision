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
layout (location = 4) in vec2 textureCoordinate;
layout (location = 0) out vec2 TexCoord;
layout (location = 1) flat out int Index;


void main()
{	
	TexCoord = textureCoordinate;
	Index = gl_DrawID;
	gl_Position = pMatrix * vMatrix * vec4(vertex.xyz - vec3(0,0,10), 1);
}