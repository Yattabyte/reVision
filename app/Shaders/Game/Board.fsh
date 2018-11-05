/* Board Shader. */
#version 460

layout (location = 0) in vec2 TexCoord;
layout (location = 0) out vec4 FragColor;

layout (binding = 0) uniform sampler2D BoardTexture;

void main()
{		
	FragColor = texture(BoardTexture, TexCoord);
	if (FragColor.a <= 0.5f)
		FragColor.a = 0.5f;
}