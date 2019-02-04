/* UI Panel Shader. */
#version 460

// Uniforms
layout (location = 1) uniform vec3 color = vec3(0.20f);

// Outputs
layout (location = 0) out vec4 FragColor;


void main()
{		
	FragColor = vec4(color, 1.0f);	
}