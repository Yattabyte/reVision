/* 3D Indicator shader. */
#version 460

layout (location = 0) in vec3 vertex;
layout (location = 1) in vec2 texcoord;
layout (location = 3) in vec3 normal;

layout (location = 0) uniform mat4 pvmMat;

layout (location = 0) out vec2 UV;
layout (location = 1) out vec3 N;

void main()
{	
	UV = texcoord;
	N = normal;
	gl_Position = pvmMat * vec4(vertex, 1.0);	
}
