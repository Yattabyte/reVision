/* Score Shader. */
#version 460
#package "Game\GameBuffer"

layout (location = 0) in vec2 TexCoord;
layout (location = 1) flat in float NumberToRender;
layout (location = 2) flat in float HighlightAmount;
layout (location = 3) flat in uint UseBackdrop;
layout (location = 0) out vec4 HeaderColor;

layout (binding = 1) uniform sampler2D Numbers;


void main()
{		
	const ivec2 Size = textureSize(Numbers, 0);
	const float ElementCount = 12.0f;	
	const float AtlasWidth = float(Size.x);
	const float ElementWidth = float(Size.x / ElementCount);
	
	const vec2 DigitIndex = vec2((TexCoord.x / ElementCount) + ((NumberToRender * ElementWidth) / AtlasWidth), TexCoord.y);
	const vec3 boardColor = colorScheme * colorScheme * (colorScheme / M_PI);
	const vec3 greenColor = vec3(0, 1.0F / M_PI, 0) * (multiplier + 1);
	vec4 DigitModifier = vec4(mix(boardColor, greenColor, HighlightAmount), 1);
	if (UseBackdrop != 0)
		DigitModifier.xyz *= 0.5f;
	
	const vec4 DigitColor = texture(Numbers, DigitIndex) * DigitModifier * ((NumberToRender >= -0.5f) ? 1.0f : 0.0f);
	HeaderColor = DigitColor * intro.powerSecondary;
}
