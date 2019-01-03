/* Board Shader. */
#version 460
#package "Game\GameBuffer"

layout (location = 0) in vec2 TexCoord;
layout (location = 1) flat in int Index;
layout (location = 2) in float Dot;
layout (location = 0) out vec4 FragColor;

layout (binding = 0) uniform sampler1D BorderTexture;
layout (binding = 1) uniform sampler2D BoardTexture;
layout (binding = 2) uniform sampler2D ScoreTexture;
layout (binding = 3) uniform sampler2D TimeTexture;


void main()
{		
	switch (Index) {
	case 0:				
		FragColor = vec4(texture(BorderTexture, TexCoord.y).xyz * Dot, 1);
		break;
	case 1:
		const vec4 grayBackground = vec4(0.125f * intro.powerOn);
		const vec4 boardTexture = texture(BoardTexture, TexCoord);
		FragColor = mix(grayBackground, boardTexture, boardTexture.a);
		break;
	case 2:
		FragColor = vec4(texture(ScoreTexture, TexCoord).xyz, 1);
		break;
	case 3:
		FragColor = vec4(texture(TimeTexture, TexCoord).xyz, 1);
		break;
	};
}

