/* Spawn shader. */
#version 460

layout (location = 0) in vec3 vertex;
layout (location = 1) in vec2 texcoord;
layout (location = 2) in uint meshID;
layout (location = 3) in vec3 normal;

layout (location = 0) uniform mat4 pvmMat;

layout (location = 0) out vec2 UV;
layout (location = 1) flat out uint axisID;
layout (location = 2) out vec3 N;

void main()
{	
	UV = texcoord;
	axisID = meshID;
	N = normal;
	gl_Position = pvmMat * vec4(vertex, 1.0);
}
