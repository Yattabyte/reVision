/* Thumbnail Copy shader - copies from texture array into a single texture. */
#version 460

layout(location = 0) in vec3 vertex;

layout (location = 0) out vec2 TexCoord;


void main()
{		
	TexCoord = (vertex.xy + vec2(1.0)) / 2.0;
	gl_Position = vec4(vertex.xyz, 1);	
}