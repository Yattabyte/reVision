/* UI Panel Shader. */
#version 460

layout (location = 0) in vec2 TexCoord;
layout (location = 1) flat in int Index;

layout (location = 0) out vec4 FragColor;

layout (location = 3) uniform vec3 colors[2];


void main()
{		
	FragColor = vec4(colors[Index], 1.0f);	
}