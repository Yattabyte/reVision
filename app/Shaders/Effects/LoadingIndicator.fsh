/* Displays a spinning circle indicating that something is loading. */
#version 460 

layout (binding = 0) uniform sampler2D Spinner;

layout (location = 0) in vec2 TexCoord;

layout (location = 4) uniform float blendAmt;

layout (location = 0) out vec4 EffectColor;


void main()
{
	EffectColor = texture(Spinner, TexCoord) * blendAmt;	
}