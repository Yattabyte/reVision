/* Tile-Scored Shader. */
#version 460
#package "Game\GameBuffer"

layout (location = 0) in vec2 TexCoord;
layout (location = 1) flat in int NumberToRender;
layout (location = 0) out vec4 FragColor;
layout (binding = 1) uniform sampler2D Numbers;


void main()
{	
	const ivec2 Size = textureSize(Numbers, 0);	
	const float ElementCount = 12.0f;	
	const float AtlasWidth = float(Size.x);
	const float ElementWidth = float(Size.x / ElementCount);
	const vec2 DigitIndex = vec2((TexCoord.x / ElementCount) + ((NumberToRender * ElementWidth) / AtlasWidth), TexCoord.y);
	FragColor = texture(Numbers, DigitIndex);
}
