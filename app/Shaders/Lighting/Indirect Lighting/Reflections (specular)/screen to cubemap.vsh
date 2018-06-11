#version 460

layout (location = 0) in vec3 vertex;

out vec3 vinput;

void main()
{		
	vinput = vertex;
}
