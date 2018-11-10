/* Tile-Scored Shader. */
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
layout (std430, binding = 9) readonly buffer ScoredMarkers {	
	ivec4 scoredCoords[];
};

layout (location = 0) in vec3 vertex;
layout (location = 0) out vec2 TexCoord;
layout (location = 1) flat out int RenderNumber;
layout (location = 2) flat out int NumberToRender;
layout (location = 3) flat out float TileLife;
layout (location = 0) uniform mat4 orthoProj;
layout (location = 4) uniform uint markerCount;

void main()
{	
	TexCoord = (vertex.xy + vec2(1.0)) / 2.0;
	RenderNumber = 0; // False
	NumberToRender = scoredCoords[gl_InstanceID % markerCount].z;
	mat4 tileTransform;
	// Draw the foreground
	if (gl_InstanceID >= markerCount) {
		RenderNumber = 1; // True		
		tileTransform = mat4(
			vec4(0.5f, 0.0, 0.0, 0.0),
			vec4(0.0, 0.5f, 0.0, 0.0),
			vec4(0.0, 0.0, 1.0, 0.0),
			vec4(((scoredCoords[gl_InstanceID % markerCount].x * 2) + 1), ((scoredCoords[gl_InstanceID % markerCount].y * 2) - 1) + heightOffset, 0.0, 1.0)
		);
	}
	// Draw the  background
	else {
		TileLife = lifeTick[(scoredCoords[gl_InstanceID % markerCount].y * 6) + scoredCoords[gl_InstanceID % markerCount].x];
		tileTransform = mat4(
			vec4(1.0, 0.0, 0.0, 0.0),
			vec4(0.0, 1.0, 0.0, 0.0),
			vec4(0.0, 0.0, 1.0, 0.0),
			vec4(((scoredCoords[gl_InstanceID % markerCount].x * 2) + 1), ((scoredCoords[gl_InstanceID % markerCount].y * 2) - 1) + heightOffset, 0.0, 1.0)
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

	gl_Position = orthoProj * scaleMat * tileTransform * vec4(vertex.x, vertex.y, 0, 1);
}