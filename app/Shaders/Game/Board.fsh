/* Board Shader. */
#version 460

layout (location = 0) in vec2 TexCoord;
layout (location = 1) flat in int Index;
layout (location = 0) out vec4 FragColor;

layout (binding = 0) uniform sampler2D BoardTexture;
layout (binding = 1) uniform sampler2D ScoreTexture;
layout (binding = 2) uniform sampler2D TimeTexture;

void main()
{		
	switch (Index) {
	case 0:
		FragColor = vec4(1.0f);
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