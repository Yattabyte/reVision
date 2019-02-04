/* UI List Shader. */
#version 460

// Inputs
layout (location = 0) flat in int ObjIndex;

// Uniforms
layout (location = 4) uniform bool enabled;
layout (location = 5) uniform vec4 colors[3] = { 
	vec4(1.0f), 						// Background
	vec4(0.60f, 0.85f, 0.95f, 0.50f), 	// Highlight
	vec4(0.30f, 0.50f, 0.75f, 0.50f) 	// Selection
};

// Outputs
layout (location = 0) out vec4 FragColor;


void main()
{		
	FragColor = colors[ObjIndex];
	if (!enabled)
		FragColor *= 0.75f;
}