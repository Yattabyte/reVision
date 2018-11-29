/* Tile-Scored Shader. */
#version 460
#package "Game\GameBuffer"

layout (location = 0) in vec2 TexCoord;
layout (location = 1) flat in int RenderNumber;
layout (location = 2) flat in int NumberToRender;
layout (location = 3) flat in float TileLife;
layout (location = 0) out vec4 FragColor;
layout (binding = 0) uniform sampler2D TileTexture;
layout (binding = 1) uniform sampler2D Numbers;


void main()
{	
	if (RenderNumber > 0) {
		const ivec2 Size = textureSize(Numbers, 0);
		const float AtlasWidth = float(Size.x);
		const float ElementWidth = float(Size.y);
		const float ElementCount = 11.0f;	
		const vec2 DigitIndex = vec2((TexCoord.x / ElementCount) + ((NumberToRender * ElementWidth) / AtlasWidth), TexCoord.y);
		FragColor = texture(Numbers, DigitIndex);
	}
	else 
		FragColor = texture(TileTexture, TexCoord) * vec4( colorScheme * calcPulseAmount(gl_FragCoord.y), 1 );	
}
