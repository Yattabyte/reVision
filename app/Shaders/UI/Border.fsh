/* UI Border Shader. */
#version 460

layout (location = 2) uniform vec3 color;

layout (location = 0) out vec4 FragColor;


void main()
{		
	FragColor = vec4(color, 1.0f);	
}