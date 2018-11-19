/* Board Shader. */
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
layout (location = 1) flat in int Index;
layout (location = 2) in float Dot;
layout (location = 0) out vec4 FragColor;

layout (binding = 0) uniform sampler2D BoardTexture;
layout (binding = 1) uniform sampler2D ScoreTexture;
layout (binding = 2) uniform sampler2D TimeTexture;

void main()
{		
	switch (Index) {
	case 0:
		const float waveAmt = 0.5f * sin((-length(gl_FragCoord.y / CameraDimensions.y) * (2.0f + (excitement * 8.0))  ) + (2.0f * (float(scoreTick) / 750.0) - 1.0f) * 3.1415f * (2.0f + (excitement * 8.0))) + 0.5f;
		const float pulseAmount = 1.0f - (0.75 * (1.0f - ((1.0f - waveAmt) * (1.0f - waveAmt))));
		const vec3 boardColor = mix(vec3(0,0.5,1), vec3(1,0,0.5), excitement);
		FragColor = vec4( (boardColor * pulseAmount * Dot) + (boardColor * pulseAmount * 0.1f), 1 );
		break;
	case 1:
		FragColor = texture(BoardTexture, TexCoord);
		break;
	case 2:
		FragColor = vec4(texture(ScoreTexture, TexCoord).xyz, 1);
		break;
	case 3:
		FragColor = vec4(texture(TimeTexture, TexCoord).xyz, 1);
		break;
	};
	if (FragColor.a <= 0.5f)
		FragColor.a = 0.5f;
}