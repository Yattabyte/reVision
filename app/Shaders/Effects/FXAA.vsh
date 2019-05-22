/* FXAA shader - applies FXAA to the input image. */
#version 460

layout (std430, binding = 2) readonly coherent buffer Camera_Buffer {	
	mat4 pMatrix;
	mat4 vMatrix;
	vec3 EyePosition;
	vec2 CameraDimensions;
};

layout(location = 0) in vec3 vertex;

layout (location = 0) out vec2 TexOffset;
layout (location = 1) out vec2 TexCoord;

void main()
{		
	TexOffset = 1.0f / CameraDimensions;
	TexCoord = (vertex.xy + vec2(1.0)) / 2.0;
	gl_Position = vec4(vertex.xyz, 1);	
}