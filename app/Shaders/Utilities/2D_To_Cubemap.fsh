#version 460

layout (binding = 0) uniform sampler2D Image;
layout (location = 0) in vec2 TexCoord;
layout (location = 0) out vec3 LightingColor;

void main(void)
{   
	LightingColor = texture(Image, TexCoord).xyz;
}