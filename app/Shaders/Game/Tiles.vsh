/* Tiles Shader. */
#version 460
#package "Game\GameBuffer"

layout (std430, binding = 2) readonly coherent buffer Camera_Buffer {		
	mat4 pMatrix;
	mat4 vMatrix;
	mat4 pMatrix_Inverse;
	mat4 vMatrix_Inverse;
	vec3 EyePosition;
	vec2 CameraDimensions;
};

layout (location = 0) in vec3 vertex;
layout (location = 0) out vec2 TexCoord;
layout (location = 1) flat out uint Type;
layout (location = 2) flat out uint TileWaiting;
layout (location = 3) flat out float LifeTick;
layout (location = 4) flat out float Excitement;
layout (location = 0) uniform mat4 orthoProj;


const float TILE_POPPING = 15.0F;

float smoothStart6(float t) 
{
	return t * t * t * t * t * t;
}

void main()
{	
	TexCoord = (vertex.xy + vec2(1.0)) / 2.0;
	Type = types[gl_InstanceID];
	TileWaiting = 0;
	
	// Container for storing transformations
	mat4 tileTransform;
		
	// Identifies background tiles
	if (gl_InstanceID < 6) 
		TileWaiting = 1;
	// Draw regular tiles (BLOCKS + BACKGROUND)
	if (gl_InstanceID < (12*6)) {
		LifeTick = lifeTick[gl_InstanceID];
		Excitement = excitement;		
		const float linearLife = clamp(LifeTick / TILE_POPPING, 0.0f, 1.0f);
		const float deathScl = mix(1.0f, 0.675f, smoothStart6(linearLife));
		tileTransform = mat4(
			vec4(deathScl, 0.0, 0.0, 0.0),
			vec4(0.0, deathScl, 0.0, 0.0),
			vec4(0.0, 0.0, 1.0, 0.0),
			vec4(((gl_InstanceID % 6) * 2) + 1, (((gl_InstanceID / 6) * 2) - 1) + heightOffset, 0.0, 1.0)
		);
	}
	// Draw player tiles
	else {
		Type = 6;
		tileTransform = mat4(
			vec4(1.0, 0.0, 0.0, 0.0),
			vec4(0.0, 1.0, 0.0, 0.0),
			vec4(0.0, 0.0, 1.0, 0.0),
			vec4(((playerCoords.x * 2) + 1) + ((gl_InstanceID % 72) * 2.0f), ((playerCoords.y * 2) - 1) + heightOffset, 0.0, 1.0)
		);
	}
	
	// Scale tiles, and apply the orthographic projection
	const float tileScale = 64.0f;
	const mat4 scaleMat = mat4(
		vec4(tileScale, 0.0, 0.0, 0.0),
		vec4(0.0, tileScale, 0.0, 0.0),
		vec4(0.0, 0.0, 1.0, 0.0),
		vec4(0.0, 0.0, 0.0, 1.0)
	);
	gl_Position = orthoProj * scaleMat * tileTransform * vec4(vertex.xy, 0, 1);
}