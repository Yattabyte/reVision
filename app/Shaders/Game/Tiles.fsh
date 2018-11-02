/* Tiles Shader. */
#version 460

layout (early_fragment_tests) in;
layout (location = 0) in vec2 TexCoord;
layout (location = 1) flat in uint Type;
layout (location = 2) flat in uint TileWaiting;
layout (location = 3) flat in float LifeTick;
layout (location = 4) flat in uint DeathTick;
layout (location = 5) flat in float Excitement;
layout (location = 0) out vec4 FragColor;
layout (binding = 0) uniform sampler2D TileTexture;
layout (binding = 1) uniform sampler2D PlayerTexture;



vec4 calcTile_Background()
{
	return vec4(0);
}

vec4 calcTile_Player()
{
	return texture(PlayerTexture, TexCoord);
}

vec4 calcTile_Regular()
{
	const vec3 tileColors[5] = vec3[](
		vec3(1,0,0),
		vec3(0,1,0),
		vec3(1,1,0),
		vec3(0,1,1),
		vec3(1,0,1)
	);
	const vec4 tileAppearance = texture(TileTexture, TexCoord);
	
	const float quarterExcitement = Excitement / 4.0f;
	const float texDistance = distance(TexCoord, vec2(0.5f));
	const float Radius = 0.75f + quarterExcitement;
	const float range = 1.0f / Radius;	
	const float brightness = 1.0f - (mod(LifeTick, 5.0f) / 5.0f); // Blinking on death
	const float colorSaturation = (1.0f - (LifeTick / 10.0f)) * brightness; // decay color on death
	const float attenuationFactor = 1.0f - (texDistance * texDistance) * (range * range) + quarterExcitement;	
	if (DeathTick >= 5u)
		return vec4(0);
	return mix(tileAppearance, vec4(tileColors[Type], 1) * tileAppearance, colorSaturation) * vec4(vec3(attenuationFactor),1);
}

void main()
{		
	switch (Type) {
		case 5:
			FragColor = calcTile_Background();
			break;
		case 6:	
			FragColor = calcTile_Player();
			break;
		default:
			FragColor = calcTile_Regular();
			break;	
	};
	// Dim the tiles
	if (TileWaiting == 1)
		FragColor.xyz /= 4.0f;
}