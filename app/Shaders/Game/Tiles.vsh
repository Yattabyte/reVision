/* Tiles Shader. */
#version 460
#package "Game\GameBuffer"

layout (location = 0) in vec3 vertex;
layout (location = 0) out vec2 TexCoord;
layout (location = 1) flat out uint Type;
layout (location = 2) flat out uint TileWaiting;
layout (location = 3) flat out float TileLifeLinear;
layout (location = 0) uniform mat4 orthoProj;


float smoothStart(float t) 
{
	return t * t * t;
}

void main()
{	
	TexCoord = (vertex.xy + vec2(1.0)) / 2.0;
	Type = types[gl_InstanceID];
	TileWaiting = 0;
	
	// Scale tiles, and apply the orthographic projection
	const float scl = 64.0f;
	const mat4 scaleMat = mat4(
		vec4(scl, 0.0, 0.0, 0.0),
		vec4(0.0, scl, 0.0, 0.0),
		vec4(0.0, 0.0, 1.0, 0.0),
		vec4(0.0, 0.0, 0.0, 1.0)
	);
	
	// Signals these tiles as 'Waiting' tiles
	if (gl_InstanceID < 6) 
		TileWaiting = 1;
	
	// Draw regular tiles (BLOCKS + BACKGROUND)
	if (gl_InstanceID < (12*6)) {
		const bool alive = lifeLinear[gl_InstanceID] <= -0.01f ? false : true;
		TileLifeLinear = clamp(lifeLinear[gl_InstanceID], 0.0F, 1.0F);
		const float scl = (1.0f - smoothStart(TileLifeLinear)) * (!alive ? 0.85f : 1.0f);
		const mat4 tileTransform = mat4(
			vec4(scl, 0.0, 0.0, 0.0),
			vec4(0.0, scl, 0.0, 0.0),
			vec4(0.0, 0.0, 1.0, 0.0),
			vec4(((gl_InstanceID % 6) * 2) + 1, (((gl_InstanceID / 6) * 2) - 1) + heightOffset - (!alive ? 0.0f : gravityOffsets[gl_InstanceID]), 0.0, 1.0)
		);		
		gl_Position = orthoProj * scaleMat * tileTransform * vec4(vertex.xy, 0, 1);
	}
	// Draw player tiles
	else {
		Type = 6;
		const mat4 tileTransform = mat4(
			vec4(1.0, 0.0, 0.0, 0.0),
			vec4(0.0, 1.0, 0.0, 0.0),
			vec4(0.0, 0.0, 1.0, 0.0),
			vec4(((playerCoords.x * 2) + 1) + ((gl_InstanceID % 72) * 2.0f), ((playerCoords.y * 2) - 1) + heightOffset, 0.0, 1.0)
		);
		gl_Position = orthoProj * scaleMat * tileTransform * vec4(vertex.xy, 0, 1);
	}	
}