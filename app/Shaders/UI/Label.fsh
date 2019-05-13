/* UI Label Shader. */
#version 460

// Inputs
layout (location = 0) in vec2 TexCoord;
layout (location = 1) flat in int Character;

// Uniforms
layout (binding = 0) uniform sampler2D FontTexture;
layout (location = 4) uniform bool enabled;
layout (location = 5) uniform vec3 color = vec3(1.0f);

// Outputs
layout (location = 0) out vec4 FragColor;


void main()
{
	// Atlas lookup logic, transforms texcoords to the specific region of the texture for this char
	const float x = float(int(Character % 11) / 11.0f);
	const float y = 1.0f - (float(int(Character / 11) / 11.0f) + (1.0f / 11.0f));
	const vec2 newTexCoord = (TexCoord / 11.0f) + vec2(x, y);
	FragColor = texture(FontTexture, newTexCoord) * vec4(color, 1);
	if (!enabled)
		FragColor *= 0.75f;
}