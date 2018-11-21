/* Stop-Timer Shader. */
#version 460
#package "Game\GameBuffer"

layout (location = 0) in vec2 TexCoord;
layout (location = 1) flat in int CharToRender;
layout (location = 0) out vec4 FooterColor;
layout (binding = 0) uniform sampler2D Text;
layout (binding = 1) uniform sampler2D Numbers;


void main()
{	
	const float blinkSpeed = 33.0f * ((1.0f - (stopTimer / 10.0f)) * (1.0f - (stopTimer / 10.0f)));
	const float waveAmt = sin( blinkSpeed * (2.0f * (float(gameTick) / 750.0f) - 1.0f) * 3.1415f );
	const float pulseAmount = (0.75f - (0.5f * (1.0f - ((1.0f - waveAmt) * (1.0f - waveAmt))))) * (stopTimer < 0 ? 0.0f : 1.0f);
	const vec3 textColor = mix(vec3(0.75f,0,0), vec3(0.25f),  waveAmt*waveAmt) * 0.75f;
	// Render Text
	if (CharToRender < 0)	
		FooterColor = texture(Text, TexCoord);	
	// Render Numbers
	else {		
		const ivec2 Size = textureSize(Numbers, 0);
		const float AtlasWidth = float(Size.x);
		const float ElementWidth = float(Size.y);
		const float ElementCount = 11.0f;

		const vec2 DigitIndex = vec2((TexCoord.x / ElementCount) + ((CharToRender * ElementWidth) / AtlasWidth), TexCoord.y);
		FooterColor = texture(Numbers, DigitIndex);
	}
	FooterColor *= (vec4(textColor, 1) * pulseAmount);
}
