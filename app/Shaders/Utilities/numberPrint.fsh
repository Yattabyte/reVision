/* Number printing shader. */
#version 460
#extension GL_ARB_bindless_texture : require

layout (location = 0) in vec2 UV;
layout (location = 0) out vec4 fragColor;
layout (location = 0, bindless_sampler) uniform sampler2D Numbers;
layout (location = 3) uniform int index;
void main()
{			
	const float AtlasWidth = 264.0f;
	const float ElementWidth = 24.0f;
	const float ElementCount = 11.0f;
	
	vec2 TexCoords = vec2((UV.x / ElementCount) + ((float(index) * ElementWidth) / AtlasWidth), UV.y);
	fragColor = texture(Numbers, TexCoords);
}
