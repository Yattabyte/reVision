/* UI Text Input Shader. */
#version 460

// Inputs
layout (location = 1) flat in int ObjIndex;

// Uniforms
layout (location = 1) uniform bool enabled;
layout (location = 2) uniform bool editEnabled;
layout (location = 3) uniform float blinkTime;
layout (location = 4) uniform vec3 colors[2] = { 
	vec3(1.0f), 				// Background
	vec3(0.0f)	 				// Caret
};

// Outputs
layout (location = 0) out vec4 FragColor;


void main()
{	
	FragColor = vec4(colors[ObjIndex], 1.0f);
	if (ObjIndex == 1 && (!editEnabled || mod(blinkTime, 1.0f) <= 0.5f))
		FragColor = vec4(0);
	if (!enabled)
		FragColor *= 0.75f;
}