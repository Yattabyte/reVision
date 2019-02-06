/* UI Text Input Shader. */
#version 460

// Inputs
layout (location = 1) flat in int ObjIndex;

// Uniforms
layout (location = 1) uniform bool enabled;
layout (location = 2) uniform vec3 colors[4] = { 
	vec3(1.0f), 				// Background
	vec3(0.20f, 0.80f, 0.40f), 	// Arrow regular
	vec3(0.40f, 0.90f, 0.60f), 	// Arrow highlighted
	vec3(0.10f, 0.70f, 0.30f) 	// Arrow pressed
};

// Outputs
layout (location = 0) out vec4 FragColor;


void main()
{	
	FragColor = vec4(colors[ObjIndex], 1.0f);
	if (!enabled)
		FragColor *= 0.75f;
}