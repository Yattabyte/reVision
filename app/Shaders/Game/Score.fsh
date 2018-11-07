/* Score Shader. */
#version 460

layout (location = 0) in vec2 TexCoord;
layout (location = 1) flat in float NumberToRender;
layout (location = 2) flat in float HighlightAmount;
layout (location = 3) flat in uint UseBackdrop;
layout (location = 0) out vec4 FragColor;

layout (binding = 0) uniform sampler2D Numbers;

void main()
{		
	const ivec2 Size = textureSize(Numbers, 0);
	const float AtlasWidth = float(Size.x);
	const float ElementWidth = float(Size.y);
	const float ElementCount = 10.0f;
	
	const vec2 DigitIndex = vec2((TexCoord.x / ElementCount) + ((NumberToRender * ElementWidth) / AtlasWidth), TexCoord.y);
	const vec2 EightIndex = vec2((TexCoord.x / ElementCount) + ((8 * ElementWidth) / AtlasWidth), TexCoord.y);
	
	vec4 DigitModifier = vec4(mix(vec3(0,1,1), vec3(0,1,0),HighlightAmount),1);
	if (UseBackdrop != 0)
		DigitModifier.xyz *= 0.25f;
	
	const vec4 DigitColor = texture(Numbers, DigitIndex) * DigitModifier * ((NumberToRender >= -0.5f) ? 1.0f : 0.0f);
	const vec4 EightColor = texture(Numbers, EightIndex) * 0.1f;
	FragColor = DigitColor*DigitColor.a + EightColor*(1.0f-DigitColor.a);
}