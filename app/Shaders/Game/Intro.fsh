/* Intro Shader. */
#version 460
#package "Game\GameBuffer"

layout (location = 0) in vec2 TexCoord;
layout (location = 0) out vec4 FragColor;

layout (binding = 0) uniform sampler2D Countdown;


void main()
{
	const ivec2 Size = textureSize(Countdown, 0);
	const float ElementCount = 6.0f;	
	const float AtlasWidth = float(Size.x);
	const float ElementWidth = float(Size.x / ElementCount);
	
	const vec2 DigitIndex = vec2((TexCoord.x / ElementCount) + ((intro.countDown * ElementWidth) / AtlasWidth), TexCoord.y);
	const vec4 DigitColor = texture(Countdown, DigitIndex) * (intro.countDown != -1 ? 1.0f : 0.0f);
	FragColor = DigitColor * vec4(1, 0.8, 0.5, 1);
	float waveAmt = 1.0f - (0.5f * (sin((sysTime*10.0f) + length(TexCoord / 2.0f) * M_PI)) + 0.5f);
	waveAmt = waveAmt * waveAmt * waveAmt;
	FragColor.xyz += vec3(waveAmt);
}