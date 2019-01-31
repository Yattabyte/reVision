/* UI Drop List Shader. */
#version 460

layout (location = 1) flat in int Index;

layout (location = 0) out vec4 FragColor;

layout (location = 3) uniform vec3 colors[2];


void main()
{		
	FragColor = vec4(colors[0], 1.0f);	
}