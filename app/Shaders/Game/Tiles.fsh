/* Tiles Shader. */
#version 460
#package "Game\GameBuffer"

layout (location = 0) in vec2 TexCoord;
layout (location = 1) flat in uint Type;
layout (location = 2) flat in uint TileWaiting;
layout (location = 3) flat in float TileLifeLinear;
layout (location = 0) out vec4 FragColor;
layout (binding = 0) uniform sampler2D TileTexture;
layout (binding = 1) uniform sampler2D PlayerTexture;


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
	// Diagonal lighting based on board activity
	const float waveAmt = 0.5f * (sin((gameWave + -length(gl_FragCoord.xy / CameraDimensions.xy)) * M_PI)) + 0.5f;
	const float maxBrightness = 1.125f;
	const float minBrightness = 0.33f;
	const float pulseAmtDiag = (maxBrightness - (minBrightness * (1.0f - ((1.0f - waveAmt) * (1.0f - waveAmt)))));
	const float attenuationFactor = (pulseAmtDiag * pulseAmtDiag * pulseAmtDiag * pulseAmtDiag) * 0.05f; 	
	
	// Tile backing appearance	
	vec4 tileAppearance = texture(TileTexture, TexCoord);
		
	// Final Appaearance
	return tileAppearance * mix(	
		vec4(tileColors[Type] * (1.0f - attenuationFactor), 1) + attenuationFactor, 
		vec4(1.0f), 
		smoothStop6(TileLifeLinear) + attenuationFactor // desaturation amount
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
	return vec4(tileColors[Type] * 0.25f * attenuationFactor, 1);
}

void main()
{		
	if (Type == 6)
		FragColor = calcTile_Player();
	else if (Type == 5)
		FragColor = vec4(0);
	else
		FragColor = calcTile_Regular();
	if (TileWaiting == 1)			
		FragColor *= 0.5f;
}