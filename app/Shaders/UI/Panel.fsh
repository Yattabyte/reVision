/* UI Panel Shader. */
#version 460

// Uniforms
layout (location = 1) uniform vec4 color = vec4(0.20f);

// Outputs
layout (location = 0) out vec4 FragColor;


void main()
{		
	FragColor = color;
}