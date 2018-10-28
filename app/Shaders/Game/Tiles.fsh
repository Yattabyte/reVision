/* Tiles Shader. */
#version 460

layout (early_fragment_tests) in;
layout (location = 0) in vec2 TexCoord;
layout (location = 1) flat in uint Type;
layout (location = 2) flat in uint TileWaiting;
layout (location = 0) out vec4 FragColor;
layout (binding = 0) uniform sampler2DArray TileTextures;

void main()
{		
	if (Type == 5)
		FragColor = vec4(0);
	else if (Type == 6)
		FragColor = texture(TileTextures, vec3(TexCoord, 5)); // player caret spot
	else
		FragColor = texture(TileTextures, vec3(TexCoord, Type));
	if (TileWaiting == 1)
		FragColor.xyz /= 4.0f;
}