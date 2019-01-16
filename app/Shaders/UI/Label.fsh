/* UI Label Shader. */
#version 460

layout (location = 0) in vec2 TexCoord;
layout (location = 1) flat in int Index;

layout (location = 0) out vec4 FragColor;

layout (binding = 0) uniform sampler2D FontTexture;

layout (std430, binding = 8) readonly buffer TextBuffer {
	int count;
	int characters[];
};


void main()
{
	const int character = characters[Index];
	const float x = float(int(character % 11) / 11.0f);
	const float y = 1.0f - (float(int(character / 11) / 11.0f) + (1.0f / 11.0f));
	const vec2 newTexCoord = (TexCoord / 11.0f) + vec2(x, y);
	FragColor = texture(FontTexture, newTexCoord);
}