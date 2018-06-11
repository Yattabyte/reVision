#version 460

layout (location = 0) out vec4 BlurColor;
layout (binding = 0) uniform sampler2D TextureMap0;
layout (binding = 1) uniform sampler2D TextureMap1;
layout (location = 0) uniform bool horizontal;
layout (location = 1) uniform vec2 Size;
const float weights[5] = float[](0.2270270270, 0.1945945946, 0.1216216216, 0.0540540541, 0.0162162162);

in vec2 TexCoord;

void main()
{			
	const ivec2 uvs = ivec2(TexCoord * Size);
	if (horizontal) {
		BlurColor.a = texelFetch(TextureMap1, uvs, 0).a * weights[0];	
		for (int i = 1; i < 5; ++i) {
			BlurColor.a += texelFetch(TextureMap1, uvs + ivec2(i, 0), 0).a * weights[i];	
			BlurColor.a += texelFetch(TextureMap1, uvs - ivec2(i, 0), 0).a * weights[i];	
		}
	}			
	else  {
		BlurColor.a = texelFetch(TextureMap0, uvs, 0).a * weights[0];	
		for (int i = 1; i < 5; ++i) {
			BlurColor.a += texelFetch(TextureMap0, uvs + ivec2(0, i), 0).a * weights[i];	
			BlurColor.a += texelFetch(TextureMap0, uvs - ivec2(0, i), 0).a * weights[i];	
		}
	}
}