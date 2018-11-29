/* Tiles Shader. */
#version 460
#package "Game\GameBuffer"

layout (location = 0) in vec2 TexCoord;
layout (location = 1) flat in uint Type;
layout (location = 2) flat in uint TileWaiting;
layout (location = 3) flat in float LifeTick;
layout (location = 0) out vec4 FragColor;
layout (binding = 0) uniform sampler2D TileTexture;
layout (binding = 1) uniform sampler2D PlayerTexture;


// Tick Rates
const float TILE_MAX_LIFE = 50.0F;
const float TILE_POPPING = 15.0F;
// Different tile type colors
const vec3 tileColors[5] = vec3[](
	vec3(1,0,0),
	vec3(0,1,0),
	vec3(1,1,0),
	vec3(0,1,1),
	vec3(1,0,1)
);

float smoothStop6(float t)
{
	return 1.0f - ((1.0f - t)*(1.0f - t)*(1.0f - t)*(1.0f - t)*(1.0f - t)*(1.0f - t));
}

float arch(float t) 
{
	return t * (1.0f - t);
}

vec4 chromaticAberration(sampler2D sampler, vec2 texCoord, float t) 
{
	const float arch = t * t * t;
	const float amt = arch * 0.04;
	const float rValue = texture(sampler, texCoord + vec2(amt, -amt)).r;
	const float gValue = texture(sampler, texCoord + vec2(-amt, amt)).g;
	const float bValue = texture(sampler, texCoord + vec2(-amt, amt)).b;
	const float aValue = texture(sampler, texCoord).a;
	return vec4(rValue, gValue, bValue, aValue);
}

vec4 calcTile_Background()
{
	return vec4(0);
}

vec4 calcTile_Player()
{
	return texture(PlayerTexture, TexCoord) * vec4( colorScheme * calcPulseAmount(gl_FragCoord.y), 1 );
}

vec4 calcTile_Regular()
{	
	const float pulseAmount = calcPulseAmount(gl_FragCoord.y);
	const float linearLife = clamp(LifeTick / TILE_POPPING, 0.0f, 1.0f);
	
	// Blinking on death
	const float brightness = clamp(mod(LifeTick, TILE_POPPING) / TILE_POPPING, 0.0f, 1.0f);
	
	// decay color on death
	const float colorSaturation = smoothStop6(linearLife);
	
	// Vary illumination amount based on board excitement
	const float quarterExcitement = excitementLinear / 4.0f;
	const float texDistance = distance(TexCoord, vec2(0.5f));
	const float range = 1.0f / (0.7f + pulseAmount);	
	const float attenuationFactor = 1.0f - (texDistance * texDistance) * (range * range); 	
	
	// Tile backing appearance	
	vec4 tileAppearance = chromaticAberration(TileTexture, TexCoord, brightness);
	tileAppearance.xyz += (((1.0f - pulseAmount) * 0.25f));
		
	// Final Appaearance
	return tileAppearance * mix(	
		vec4(tileColors[Type] * attenuationFactor, 1), 
		vec4(1.0f), 
		colorSaturation * brightness + ((0.1 * pulseAmount))
	);
}

vec4 calcTile_Waiting()
{
	// Vary illumination amount based on board excitementLinear
	const float quarterExcitement = excitementLinear / 4.0f;
	const float texDistance = distance(TexCoord, vec2(0.5f));
	const float Radius = 0.75f + quarterExcitement;
	const float range = 1.0f / Radius;	
	const float attenuationFactor = 1.0f - (texDistance * texDistance) * (range * range) + quarterExcitement; 
	
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