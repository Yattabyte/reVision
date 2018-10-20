/* Gaussian Blur - alpha - shader. */
#version 460
#pragma optionNV(unroll all)

layout (location = 0) out float BlurColor;
layout (binding = 0) uniform sampler2D TextureMap0;
layout (binding = 1) uniform sampler2D TextureMap1;
layout (location = 0) uniform bool horizontal;
layout (location = 1) uniform vec2 Size;
const float weights[5] = float[](0.2270270270, 0.1945945946, 0.1216216216, 0.0540540541, 0.0162162162);

layout (location = 0) in vec2 TexCoord;

void main()
{			
	const ivec2 uvs = ivec2(TexCoord * Size);
	if (horizontal) {
		BlurColor = texelFetch(TextureMap1, uvs, 0).r * weights[0];	
		for (int i = 1; i < 5; ++i) {
			BlurColor += texelFetch(TextureMap1, uvs + ivec2(i, 0), 0).r * weights[i];	
			BlurColor += texelFetch(TextureMap1, uvs - ivec2(i, 0), 0).r * weights[i];	
		}
	}			
	else  {
		BlurColor = texelFetch(TextureMap0, uvs, 0).r * weights[0];	
		for (int i = 1; i < 5; ++i) {
			BlurColor += texelFetch(TextureMap0, uvs + ivec2(0, i), 0).r * weights[i];	
			BlurColor += texelFetch(TextureMap0, uvs - ivec2(0, i), 0).r * weights[i];	
		}
	}
}