/* Stop-Timer Shader. */
#version 460

layout (location = 0) in vec2 TexCoord;
layout (location = 1) flat in int CharToRender;
layout (location = 0) out vec4 FooterColor;
layout (binding = 0) uniform sampler2D Numbers;


void main()
{	
	const ivec2 Size = textureSize(Numbers, 0);
	const float AtlasWidth = float(Size.x);
	const float ElementWidth = float(Size.y);
	const float ElementCount = 11.0f;

	const vec2 DigitIndex = vec2((TexCoord.x / ElementCount) + ((CharToRender * ElementWidth) / AtlasWidth), TexCoord.y);
	FooterColor = texture(Numbers, DigitIndex) * vec4(1,0,0,1);
}
