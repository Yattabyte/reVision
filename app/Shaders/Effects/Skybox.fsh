/* Calculates a 2D skybox from a quad. */
#version 460

layout (early_fragment_tests) in;
layout (binding = 4) uniform samplerCube SkyMap;
layout (location = 0) out vec3 LightingColor;

layout (location = 0) in vec3 vecView;

void main()
{		
	LightingColor = pow(textureLod(SkyMap, vecView, 0).rgb, vec3(2.2f));
}