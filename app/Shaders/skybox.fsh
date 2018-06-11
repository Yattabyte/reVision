#version 460
#package "camera"

layout (early_fragment_tests) in;
layout(location = 0) uniform samplerCube cubemapTexture;
layout (location = 0) out vec3 LightingColor;

in vec3 vecView;

void main()
{		
	LightingColor = pow(texture(cubemapTexture, vecView).rgb, vec3(2.2f));
}