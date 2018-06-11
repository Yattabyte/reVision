#version 460

layout (location = 0) uniform sampler2D screenTexture;
out vec3 ColorLod;

in vec2 TexCoord;
in flat int Read_Index;

void main()
{		
	ColorLod = textureLod(screenTexture, TexCoord, Read_Index).rgb;
}