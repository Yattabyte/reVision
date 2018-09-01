/* Screen space reflection shader - part 2 - Screen lookup from UV's */
#version 460

layout (location = 0) in vec3 vertex;

layout (location = 0) out vec2 TexCoord;

void main(void)
{	
	TexCoord = (vertex.xy + vec2(1.0)) / 2.0;
	gl_Position = vec4(vertex.xyz, 1);
}

