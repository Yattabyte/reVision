/* Border Shader. */
#version 460
#package "Game\GameBuffer"

layout (location = 0) in vec2 TexCoord;
layout (location = 0) out vec4 FragColor;
layout (location = 0) uniform float LevelLinear = 0.0f;
layout (location = 1) uniform float LevelUpMix = 0.0f;

float easeOutBounce(float t) {
	if (t < (1.0f / 2.75f))
		return (7.5625f * t *t);
	else if (t < (2.0f / 2.75f))
		return (7.5625f * (t -= (1.5f / 2.75f)) * t + .75f);
	else if (t < (2.5f / 2.75f))
		return (7.5625f * (t -= (2.25f / 2.75f)) * t + .9375f);
	else
		return (7.5625f * (t -= (2.625f / 2.75f)) * t + .984375f);
}

float easeInBounce(float t) {
	return 1.0f - easeOutBounce(1.0f - t);
}

void main()
{	
	// Used to achieve a stripe effect when the board is nearing the top
	const float stripeAmount = (nearingTop < (8.0f/11.0f)) ? 1.0f : floor(fract((TexCoord.x - (heightOffset/4.0f)) * 10.0f) + 0.5f) * nearingTop;
	// Pulses in response to game events
	const float pulseAmount = calcPulseAmount(gl_FragCoord.y) * stripeAmount;
	// Border beneath level mark is in game color, rest is white
	const vec3 color = ((TexCoord.x < LevelLinear) ? colorScheme : mix(vec3(1.0f), vec3(0,1.0f,0), LevelUpMix)) * pulseAmount;	
	// Add it all together
	const vec3 diffuseColor = (color * color) / M_PI;
	const vec3 highlightColor = color * color * excitementLinear * (multiplier/4.0f);
	const float dist = (LevelLinear - TexCoord.x) * 200.0f;
	const vec3 lineColor = vec3(clamp(1.0f - (dist * dist) * (0.25f), 0.0f, 1.0f));	
	FragColor = vec4(diffuseColor + highlightColor + lineColor, 1.0f);
	
	// Multiply by linear amount, to flicker the border on/off (like in intro/outer)
	FragColor.xyz *= easeInBounce(introAnimLinear);
}