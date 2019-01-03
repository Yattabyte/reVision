/* Border Shader. */
#version 460
#package "Game\GameBuffer"

layout (location = 0) in vec2 TexCoord;
layout (location = 0) out vec4 FragColor;
layout (location = 0) uniform float LevelLinear = 0.0f;
layout (location = 1) uniform float LevelUpMix = 0.0f;


void main()
{	
	// Used to achieve a stripe effect when the board is nearing the top
	const float stripeAmount = (nearingTop < (8.0f/11.0f)) ? 1.0f : floor(fract((TexCoord.x - (heightOffset/4.0f)) * 10.0f) + 0.5f) * nearingTop;
	// Border beneath level mark is in game color, rest is white
	const vec3 color = ((TexCoord.x < LevelLinear) ? colorScheme : mix(vec3(1.0f), vec3(0,1.0f,0), LevelUpMix));	
	// Add it all together
	const vec3 diffuseColor = (color * color) / M_PI;
	const vec3 highlightColor = color * color * (multiplier/4.0f);
	const float dist = (LevelLinear - TexCoord.x) * 200.0f;
	const vec3 lineColor = vec3(clamp(1.0f - (dist * dist) * (0.25f), 0.0f, 1.0f));	
	FragColor = vec4(diffuseColor + highlightColor + lineColor + music.beat, 1.0f) * intro.powerOn;
}