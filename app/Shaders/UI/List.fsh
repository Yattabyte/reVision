/* UI List Shader. */
#version 460

layout (location = 1) flat in int Index;

layout (location = 0) out vec4 FragColor;

layout (location = 4) uniform vec4 colors[3] = { 
	vec4(1.0f), 						// Background
	vec4(0.60f, 0.85f, 0.95f, 0.50f), 	// Highlight
	vec4(0.30f, 0.50f, 0.75f, 0.50f) 	// Selection
};


void main()
{		
	FragColor = colors[Index];	
}