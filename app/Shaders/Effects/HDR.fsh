/* HDR shader - Tone maps and gamma corrects an input image. */
#version 460

layout (location = 0) in vec2 TexCoord;
layout (location = 0) out vec3 HDRColor;
layout (binding = 0) uniform sampler2DArray LightMap;
layout (location = 0) uniform float Exposure = 1.0f;
layout (location = 1) uniform float Gamma = 1.0f;

void main()
{		
	// Acquire target data
	const vec3 Lighting			= texture(LightMap, vec3(TexCoord, gl_Layer)).rgb;	
		
	// Tone mapping
	const vec3 ToneMapped 		= vec3(1.0f) - exp(-Lighting * Exposure);
	
	// Gamma correct
    HDRColor 					= pow(ToneMapped, vec3(1.0f / Gamma));
}