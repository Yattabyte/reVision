#version 460

layout (location = 0) uniform sampler2D screenTexture;
layout (location = 0) out vec3 ColorLod;

layout (location = 0) in vec2 TexCoord;
layout (location = 1) flat in int Read_Index;

void main()
{		
	ColorLod = textureLod(screenTexture, TexCoord, Read_Index).rgb;
}