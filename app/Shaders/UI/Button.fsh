/* UI Button Shader. */
#version 460

layout (location = 0) out vec4 FragColor;

layout (location = 3) uniform vec3 color;


void main()
{		
	FragColor = vec4(color, 1.0f);	
}