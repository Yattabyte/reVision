/* Score Shader. */
#version 460
#package "Game\GameBuffer"

layout (std430, binding = 2) readonly coherent buffer Camera_Buffer {		
	mat4 pMatrix;
	mat4 vMatrix;
	mat4 pMatrix_Inverse;
	mat4 vMatrix_Inverse;
	vec3 EyePosition;
	vec2 CameraDimensions;
};

layout (location = 0) in vec2 TexCoord;
layout (location = 1) flat in float NumberToRender;
layout (location = 2) flat in float HighlightAmount;
layout (location = 3) flat in uint UseBackdrop;
layout (location = 0) out vec4 FragColor;

layout (binding = 0) uniform sampler2D Numbers;


const float SCORE_ROTATE_TICK = 750.0F;

void main()
{		
	const ivec2 Size = textureSize(Numbers, 0);
	const float AtlasWidth = float(Size.x);
	const float ElementWidth = float(Size.y);
	const float ElementCount = 10.0f;
	
	const vec2 DigitIndex = vec2((TexCoord.x / ElementCount) + ((NumberToRender * ElementWidth) / AtlasWidth), TexCoord.y);
		
	vec4 DigitModifier = vec4(mix( mix(vec3(0,0.5,1), vec3(1,0,0.5), sin(((gl_FragCoord.x / CameraDimensions.x) * 6.0f) + (2.0f * (float(scoreTick) / SCORE_ROTATE_TICK) - 1.0f) * 3.1415f)), vec3(0,1,0),HighlightAmount),1);
	if (UseBackdrop != 0)
		DigitModifier.xyz *= 0.25f;
	
	const vec4 DigitColor = texture(Numbers, DigitIndex) * DigitModifier * ((NumberToRender >= -0.5f) ? 1.0f : 0.0f);
	FragColor = DigitColor;
}