/* Combo Shader. */
#version 460

layout (location = 0) in vec2 TexCoord;
layout (location = 0) out vec4 FragColor;

layout (binding = 0) uniform sampler2D Numbers;

void main()
{		
	FragColor = vec4(1,0,0,1);
}