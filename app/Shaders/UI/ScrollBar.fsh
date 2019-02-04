/* UI Scrollbar Shader. */
#version 460

// Inputs
layout (location = 0) in vec2 TexCoord;

// Uniforms
layout (location = 1) uniform vec3 color =  vec3(0.20f, 0.30f, 0.40f);

// Outputs
layout (location = 0) out vec4 FragColor;


void main()
{		
	FragColor = vec4(color, 1.0f);	
}