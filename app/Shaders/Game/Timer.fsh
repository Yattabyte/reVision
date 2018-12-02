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
		const float ElementCount = 12.0f;	
		const float AtlasWidth = float(Size.x);
		const float ElementWidth = float(Size.x / ElementCount);
		
		const vec2 DigitIndex = vec2((TexCoord.x / ElementCount) + ((CharToRender * ElementWidth) / AtlasWidth), TexCoord.y);
		FooterColor = texture(Numbers, DigitIndex);
	}
	FooterColor *= vec4(colorScheme, 1) * timeAnimLinear;
	if (stopTimer < 0) 
		FooterColor.xyz *= calcPulseAmount(gl_FragCoord.y);
	else {
		const float blinkSpeed = 10.0f * ((1.0f - (stopTimer / 10.0f)) * (1.0f - (stopTimer / 10.0f)));
		const float waveAmt = sin( blinkSpeed * gameWave * M_PI );
		const float pulseAmount = (1.0f - ((1.0f - ((1.0f - waveAmt) * (1.0f - waveAmt)))));
		FooterColor.xyz *= vec3(1.0, 0.0, 0.0) * pulseAmount;	
	}
}
