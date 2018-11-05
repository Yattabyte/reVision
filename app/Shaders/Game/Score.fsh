/* Score Shader. */
#version 460

layout (location = 0) in vec2 TexCoord;
layout (location = 1) flat in float NumberToRender;
layout (location = 2) flat in float QuadIndex;
layout (location = 3) flat in float HighlightIndex;
layout (location = 0) out vec4 FragColor;

layout (binding = 0) uniform sampler2D Numbers;

void main()
{		
	const ivec2 Size = textureSize(Numbers, 0);
	const float AtlasWidth = float(Size.x);
	const float ElementWidth = float(Size.y);
	const float ElementCount = 11.0f;
	
	const vec2 DigitIndex = vec2((TexCoord.x / ElementCount) + ((NumberToRender * ElementWidth) / AtlasWidth), TexCoord.y);
	const vec2 EightIndex = vec2((TexCoord.x / ElementCount) + ((8 * ElementWidth) / AtlasWidth), TexCoord.y);
	
	vec4 Color = vec4(0,1,1,1);
	if ((HighlightIndex < 8.0f) && (QuadIndex >= HighlightIndex))
		Color = vec4(0,1,0,1);
	
	const vec4 DigitColor = texture(Numbers, DigitIndex) * Color * ((NumberToRender >= -0.5f) ? 1.0f : 0.0f);
	const vec4 EightColor = texture(Numbers, EightIndex) * 0.1f;
	FragColor = DigitColor*DigitColor.a + EightColor*(1.0f-DigitColor.a);
}