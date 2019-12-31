/* Axis lines shader. */
#version 460

layout (location = 0) in vec3 vertex;

layout (location = 0) uniform mat4 pvmmatrix;


void main()
{	
	gl_Position = pvmmatrix * vec4(vertex, 1.0);	
}

