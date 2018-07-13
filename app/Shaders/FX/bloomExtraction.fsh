#version 460

layout (location = 0) uniform sampler2D LightTexture;
layout (location = 0) out vec3 BloomColor;
layout (location = 0) in vec2 TexCoord;

void main()
{	
	BloomColor					= vec3(0.0f);
	const vec3 LightingColor	= texture(LightTexture, TexCoord).rgb;
	if (dot(LightingColor, vec3(0.2126, 0.7152, 0.0722)) > 1.0)
		BloomColor 				= LightingColor;//min(LightingColor, vec3(0.2126, 0.7152, 0.0722) * 8);	
}
