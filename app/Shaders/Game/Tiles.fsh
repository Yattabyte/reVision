/* Tiles Shader. */
#version 460
#package "Game\GameBuffer"

layout (location = 0) in vec2 TexCoord;
layout (location = 1) flat in uint Type;
layout (location = 2) flat in uint TileWaiting;
layout (location = 3) flat in float TileLifeLinear;
layout (location = 4) flat in float LaneAmt;
layout (location = 0) out vec4 FragColor;
layout (binding = 0) uniform sampler2D TileTexture;
layout (binding = 1) uniform sampler2D PlayerTexture;
layout (location = 4) uniform float Time;


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

float hash(float n) { return fract(sin(n) * 1e4); }

float noise(float x) {
	float i = floor(x);
	float f = fract(x);
	float u = f * f * (3.0 - 2.0 * f);
	return mix(hash(i), hash(i + 1.0), u);
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
	const float waveAmt = 0.5f * (sin((gameWave + -length(gl_FragCoord.xy / CameraDimensions.xy)) * M_PI)) + 0.5f;
	const float maxBrightness = 1.125f;
	const float minBrightness = 0.33f;
	const float pulseAmtDiag = (maxBrightness - (minBrightness * (1.0f - ((1.0f - waveAmt) * (1.0f - waveAmt)))));
	
	// Tile backing appearance	
	vec4 appearance;
	if (LaneAmt > 0.0f) {
		const float DistortionScale = noise(Time) * 0.75f;
		const float noiseAmt = (noise(gl_FragCoord.x) + noise(gl_FragCoord.y));
		const vec2 moddedTex = vec2(
			TexCoord.x,
			//TexCoord.y * ( TexCoord.y * (0.005 / (noise(gl_FragCoord.y) * DistortionScale)))
			mix((1.0f - TexCoord.y), (1.0f - TexCoord.y) * (noise(CameraDimensions.y - gl_FragCoord.y)) * DistortionScale, LaneAmt)
		);		
		appearance = texture(TileTexture, moddedTex) * vec4(mix(tileColors[Type], tileColors[clamp(int(noiseAmt * 5), 0, 4)], DistortionScale * LaneAmt * 0.5f), 1);
	}
	else 
		appearance = vec4(tileColors[Type], 1);
	appearance *= texture(TileTexture, TexCoord);
	
	// Desaturate based on life
	vec4 grayXfer = vec4(0.3, 0.59, 0.11, 1.0f);
	vec4 gray = vec4(dot(grayXfer, appearance));
	return vec4(mix(appearance, gray, smoothStop6(TileLifeLinear)).xyz, appearance.a);
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