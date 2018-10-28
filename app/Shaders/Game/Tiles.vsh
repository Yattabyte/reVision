/* Tiles Shader. */
#version 460

layout (std430, binding = 2) readonly coherent buffer Camera_Buffer {		
	mat4 pMatrix;
	mat4 vMatrix;
	mat4 pMatrix_Inverse;
	mat4 vMatrix_Inverse;
	vec3 EyePosition;
	vec2 CameraDimensions;
};
layout (std430, binding = 8) readonly buffer BoardBuffer {		
	mat4 tileMats[12*6];
	uint types[12*6];
	mat4 boardMat;
	uint tick;
	mat4 playerMat;
};
layout (location = 0) in vec3 vertex;
layout (location = 0) out vec2 TexCoord;
layout (location = 1) flat out uint Type;
layout (location = 2) flat out uint TileWaiting;
layout (location = 0) uniform mat4 orthoProj;

void main()
{	
	TexCoord = (vertex.xy + vec2(1.0)) / 2.0;
	Type = types[gl_InstanceID];
	TileWaiting = 0;
	
	// Identifies background tiles
	if (gl_InstanceID < 6) 
		TileWaiting = 1;
	// Draw regular tiles (BLOCKS + BACKGROUND)
	if (gl_InstanceID < (12*6))
		gl_Position = orthoProj * tileMats[gl_InstanceID] * vec4(vertex.x, vertex.y + (tick/16.0f) * 2.0f, 0, 1);
	// Draw player tiles
	else {
		Type = 6;
		gl_Position = orthoProj * playerMat * vec4(vertex.x + ((gl_InstanceID %(12*6)) * 2.0f), vertex.y + (tick/16.0f) * 2.0f, 0, 1);
	}
}