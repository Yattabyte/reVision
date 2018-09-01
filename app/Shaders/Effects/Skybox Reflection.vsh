/* Calculates skybox reflections. */
#version 460

layout (location = 0) in vec3 vertex;

layout (location = 0) out vec2 TexCoord;

void main()
{	
	TexCoord 		= (vertex.xy) * 0.5f + 0.5f;
	gl_Position 	= vec4(vertex,1.0);	
}