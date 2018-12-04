/* Matched Tiles Shader. */
#version 460
#package "Game\GameBuffer"

struct ScoredStruct {
	ivec2 coords;
	uint pieceStates[16];
};
layout (std430, binding = 9) readonly buffer ScoredMarkers {	
	ScoredStruct matchedData[];
};

layout (location = 0) in vec3 vertex;
layout (location = 0) out vec2 TexCoord;
layout (location = 0) uniform mat4 orthoProj;


void main()
{	
	const int index = gl_InstanceID / 16;
	TexCoord = ((vertex.xy + vec2(1.0)) / 2.0) / vec2(4.0, 3);
	const float xOffset = (2.0f * (((gl_InstanceID % 16) % 4) * 0.25f) - 1.0f) + 0.25f;
	const float yOffset = (2.0f * (((gl_InstanceID % 16) / 4) * 0.25f) - 1.0f) + 0.25f;
	const mat4 tileTransform = mat4(
		vec4(0.25, 0.0, 0.0, 0.0),
		vec4(0.0, 0.25, 0.0, 0.0),
		vec4(0.0, 0.0, 1.0, 0.0),
		vec4((matchedData[index].coords.x * 2) + 1 + xOffset, (matchedData[index].coords.y * 2) - 1 + yOffset + heightOffset, 0.0, 1.0)
	);
	const float tileScale = 64.0f;
	const mat4 scaleMat = mat4(
		vec4(tileScale, 0.0, 0.0, 0.0),
		vec4(0.0, tileScale, 0.0, 0.0),
		vec4(0.0, 0.0, 1.0, 0.0),
		vec4(0.0, 0.0, 0.0, 1.0)
	);	
	TexCoord.x += float(int(matchedData[index].pieceStates[gl_InstanceID % 16]) % 4) / 4.0f; // 0-3 on x axis
	TexCoord.y += float(int(matchedData[index].pieceStates[gl_InstanceID % 16]) / 4) / 3.0f; // 0-3 on y axis
	if (matchedData[index].pieceStates[gl_InstanceID % 16] == 16u)
		TexCoord = vec2(0.0f);
	gl_Position = orthoProj * scaleMat * tileTransform * vec4(vertex.xy, 0, 1);
}
