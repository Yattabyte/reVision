/* Timer Shader. */
#version 460
#package "Game\GameBuffer"

layout (location = 0) in vec2 TexCoord;
layout (location = 1) flat in int CharToRender;
layout (location = 0) out vec4 FooterColor;
layout (binding = 0) uniform sampler2D Text;
layout (binding = 1) uniform sampler2D Numbers;


void main()
{	
	// Render Text
	if (CharToRender < 0)	
		FooterColor = texture(Text, TexCoord);	
	// Render Numbers
	else {		
		const ivec2 Size = textureSize(Numbers, 0);
		const float AtlasWidth = float(Size.x);
		const float ElementWidth = float(Size.y);
		const float ElementCount = 12.0f;
		
		const vec2 DigitIndex = vec2((TexCoord.x / ElementCount) + ((CharToRender * ElementWidth) / AtlasWidth), TexCoord.y);
		FooterColor = texture(Numbers, DigitIndex);
	}
	float waveAmt = 0.5f * sin((-length(gl_FragCoord.y / 128) * (2.0f + (excitement * 8.0))  ) + (2.0f * (float(gameTick) / 750.0) - 1.0f) * 3.1415f * (2.0f + (excitement * 8.0))) + 0.5f;
	if (stopTimer >= 0) {
		const float blinkSpeed = 33.0f * ((1.0f - (stopTimer / 10.0f)) * (1.0f - (stopTimer / 10.0f)));
		waveAmt = sin( blinkSpeed * (2.0f * (float(gameTick) / 750.0f) - 1.0f) * 3.1415f );
	}
	const float pulseAmount = (0.75f - (0.25f * (1.0f - ((1.0f - waveAmt) * (1.0f - waveAmt)))));
	const vec3 textColor = mix(vec3(1.0), vec3(1.0, 0.5, 0.0),  pulseAmount*pulseAmount);	
	FooterColor *= vec4(textColor, 1) * animTime;
}
