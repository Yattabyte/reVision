/* Editor Screen Copy shader. */
#version 460

layout (location = 0) in vec3 vertex;
layout (location = 1) in vec2 texcoord;

layout (location = 0) out vec2 UV;

void main()
{	
	UV = texcoord;
	gl_Position = vec4(vertex, 1.0);	
}

