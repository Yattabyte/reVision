/* Tiles Shader. */
#version 460

layout (early_fragment_tests) in;
layout (location = 0) in vec2 TexCoord;
layout (location = 1) flat in uint Type;
layout (location = 2) flat in uint TileWaiting;
layout (location = 3) flat in float Brightness;
layout (location = 4) flat in float Excitement;
layout (location = 0) out vec4 FragColor;
layout (binding = 0) uniform sampler2D TileTexture;
layout (binding = 1) uniform sampler2D PlayerTexture;

const vec3 tileColors[5] = vec3[](
	vec3(1,0,0),
	vec3(0,1,0),
	vec3(1,1,0),
	vec3(0,1,1),
	vec3(1,0,1)
);

void main()
{		
	// Background Tiles
	if (Type == 5)
		FragColor = vec4(0);
	// Player Tiles
	else if (Type == 6)
		FragColor = texture(PlayerTexture, TexCoord);
	// Regular Tiles (A-E)
	else {
		const float quarterExcitement = Excitement / 4.0f;
		const float texDistance = distance(TexCoord, vec2(0.5f));
		const float Radius = 0.75f + quarterExcitement;
		const float range = 1.0f / Radius;
		const float attenuationFactor = 1.0f - (texDistance * texDistance) * (range * range) * Brightness + quarterExcitement;
		FragColor = vec4(tileColors[Type], 1) * texture(TileTexture, TexCoord) * attenuationFactor;
	}
	// Dim the tiles
	if (TileWaiting == 1)
		FragColor.xyz /= 4.0f;
}