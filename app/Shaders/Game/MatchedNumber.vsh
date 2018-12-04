/* Tile-Scored Shader. */
#version 460
#package "Game\GameBuffer"

struct CountStruct {
	vec2 center;
	ivec2 count;
};
layout (std430, binding = 9) readonly buffer ScoredCounts {	
	CountStruct matchedData[];
};

layout (location = 0) in vec3 vertex;
layout (location = 0) out vec2 TexCoord;
layout (location = 1) flat out int NumberToRender;
layout (location = 0) uniform mat4 orthoProj;


void main()
{	
	TexCoord = (vertex.xy + vec2(1.0f)) / 2.0f;
	NumberToRender = matchedData[gl_InstanceID].count.x;
	const float scl = 0.5F;
	const mat4 tileTransform = mat4(
		vec4(scl, 0.0, 0.0, 0.0),
		vec4(0.0, scl, 0.0, 0.0),
		vec4(0.0, 0.0, 1.0, 0.0),
		vec4(((matchedData[gl_InstanceID].center.x * 2.0f) + 1.0f), ((matchedData[gl_InstanceID].center.y * 2.0f) - 1.0f) + heightOffset, 0.0f, 1.0f)
	);
	const mat4 scaleMat = mat4(
		vec4(64, 0.0, 0.0, 0.0),
		vec4(0.0, 64, 0.0, 0.0),
		vec4(0.0, 0.0, 1.0, 0.0),
		vec4(0.0, 0.0, 0.0, 1.0)
	);	
	gl_Position = orthoProj * scaleMat * tileTransform * vec4(vertex.xy, 0, 1);
}