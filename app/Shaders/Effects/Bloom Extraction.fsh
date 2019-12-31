/* Extracts bloom features from an image. */
#version 460

layout (location = 0) uniform sampler2DArray LightTexture;
layout (location = 0) out vec3 BloomColor;
layout (location = 0) in vec2 TexCoord;

void main()
{	
	BloomColor					= vec3(0.0f);
	const vec3 LightingColor	= texture(LightTexture, vec3(TexCoord, gl_Layer)).rgb;
	if (dot(LightingColor, vec3(0.2126, 0.7152, 0.0722)) > 1.0)
		BloomColor 				= LightingColor;
}
