/* Gaussian Blur - MIP - shader. */
#version 460
#pragma optionNV(fastmath on)
#pragma optionNV(fastprecision on)
#pragma optionNV(ifcvt none)
#pragma optionNV(inline all)
#pragma optionNV(strict on)
#pragma optionNV(unroll all)

layout (location = 0) out vec3 BlurColor;
layout (binding = 0) uniform sampler2DArray TextureMap;
layout (location = 0) uniform bool horizontal;
layout (location = 1) uniform ivec2 Size;
const float weights[5] = float[](0.2270270270, 0.1945945946, 0.1216216216, 0.0540540541, 0.0162162162);

layout (location = 0) in vec2 TexCoord;

void main()
{			
	// We force the mip level through texture parameters, so here we can specify it as 0.
	const ivec3 uvs = ivec3(TexCoord * Size, gl_Layer);
	BlurColor = texelFetch(TextureMap, uvs, 0).rgb * weights[0];	
	if (horizontal) 
		for (int i = 1; i < 5; ++i) {
			BlurColor += texelFetch(TextureMap, uvs + ivec3(i, 0, 0), 0).rgb * (weights[i] * 0.5);	
			BlurColor += texelFetch(TextureMap, uvs - ivec3(i, 0, 0), 0).rgb * (weights[i] * 0.5);	
		}
	else 
		for (int i = 1; i < 5; ++i) {
			BlurColor += texelFetch(TextureMap, uvs + ivec3(0, i, 0), 0).rgb * (weights[i] * 0.5);	
			BlurColor += texelFetch(TextureMap, uvs - ivec3(0, i, 0), 0).rgb * (weights[i] * 0.5);	
		}
}