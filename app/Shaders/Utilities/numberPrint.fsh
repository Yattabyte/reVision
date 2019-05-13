/* Number printing shader. */
#version 460

layout (location = 0) in vec2 UV;
layout (location = 0) out vec4 fragColor;
layout (location = 3) uniform int index;

layout (binding = 0) uniform sampler2D Numbers;


void main()
{			
	const float AtlasWidth = 264.0f;
	const float ElementWidth = 24.0f;
	const float ElementCount = 11.0f;
	
	vec2 TexCoords = vec2((UV.x / ElementCount) + ((float(index) * ElementWidth) / AtlasWidth), UV.y);
	fragColor = texture(Numbers, TexCoords);
}
