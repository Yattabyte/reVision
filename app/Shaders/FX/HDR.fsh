#version 460
#package "camera"

layout (location = 0) out vec3 HDRColor;
layout (binding = 0) uniform sampler2D LightMap;
layout (binding = 1) uniform sampler2D BloomMap;
layout (location = 0) uniform float Exposure = 1.0;

in vec2 TexCoord;

void main()
{		
	// Acquire target data
	const vec3 Lighting			= texture(LightMap, TexCoord).rgb + texture(BloomMap, TexCoord).rgb;	
		
	// Tone mapping
	const vec3 ToneMapped 		= vec3(1.0f) - exp(-Lighting * Exposure);
	
	// Gamma correct
    HDRColor 					= pow(ToneMapped, vec3(1.0f / Gamma));
}