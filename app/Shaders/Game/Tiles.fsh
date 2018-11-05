/* Tiles Shader. */
#version 460

layout (location = 0) in vec2 TexCoord;
layout (location = 1) flat in uint Type;
layout (location = 2) flat in uint TileWaiting;
layout (location = 3) flat in float LifeTick;
layout (location = 4) flat in float Excitement;
layout (location = 0) out vec4 FragColor;
layout (binding = 0) uniform sampler2D TileTexture;
layout (binding = 1) uniform sampler2D PlayerTexture;


const float TILE_MAX_LIFE = 50.0F;
const float TILE_POPPING = 15.0F;

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
	// Blinking on death
	const float brightness = 1.0f - clamp(mod(LifeTick, TILE_POPPING) / TILE_POPPING, 0.0f, 1.0f);
	
	// decay color on death
	const float colorSaturation = (1.0f - clamp(LifeTick / (TILE_MAX_LIFE * 2.0F), 0.0f, 1.0f)) * brightness;
	
	// Vary illumination amount based on board excitement
	const float quarterExcitement = Excitement / 4.0f;
	const float texDistance = distance(TexCoord, vec2(0.5f));
	const float Radius = 0.75f + quarterExcitement;
	const float range = 1.0f / Radius;	
	const float attenuationFactor = 1.0f - (texDistance * texDistance) * (range * range) + quarterExcitement; 	
	
	// Tile backing appearance	
	const vec4 tileAppearance = texture(TileTexture, TexCoord);
	
	// Different tile type colors
	const vec3 tileColors[5] = vec3[](
		vec3(1,0,0),
		vec3(0,1,0),
		vec3(1,1,0),
		vec3(0,1,1),
		vec3(1,0,1)
	);
	
	// Final Appaearance
	return mix(tileAppearance, vec4(tileColors[Type], 1) * tileAppearance, colorSaturation) * vec4(vec3(attenuationFactor),1);
}

vec4 calcTile_Waiting()
{
	// Vary illumination amount based on board excitement
	const float quarterExcitement = Excitement / 4.0f;
	const float texDistance = distance(TexCoord, vec2(0.5f));
	const float Radius = 0.75f + quarterExcitement;
	const float range = 1.0f / Radius;	
	const float attenuationFactor = 1.0f - (texDistance * texDistance) * (range * range) + quarterExcitement; 
	
	// Different tile type colors
	const vec3 tileColors[5] = vec3[](
		vec3(1,0,0),
		vec3(0,1,0),
		vec3(1,1,0),
		vec3(0,1,1),
		vec3(1,0,1)
	);
	
	// Final Appaearance
	return vec4(tileColors[Type] * 0.5f * attenuationFactor, 1);
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
			if (TileWaiting == 1)			
				FragColor = calcTile_Waiting();
			else
				FragColor = calcTile_Regular();
			break;	
	};
	// Dim the tiles
	if (TileWaiting == 1)
		FragColor.xyz /= 4.0f;
}