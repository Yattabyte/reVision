#version 460

layout (location = 0) in vec3 vertex;

layout (location = 0) out vec3 vinput;

void main()
{		
	vinput = vertex;
}
