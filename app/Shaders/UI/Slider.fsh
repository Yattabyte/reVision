/* UI Slider Shader. */
#version 460

// Inputs
layout (location = 0) flat in int ObjIndex;

// Uniforms
layout (location = 1) uniform bool enabled;
layout (location = 2) uniform bool highlighted;
layout (location = 3) uniform bool pressed;
layout (location = 4) uniform vec3 colors[5] = { 
	vec3(0.20f, 0.30f, 0.40f),		// background
	vec3(0.15f, 0.60f, 0.30f), 		// trail
	vec3(0.20f, 0.80f, 0.40f), 		// regular
	vec3(0.40f, 0.90f, 0.60f), 		// highlighted
	vec3(0.10f, 0.70f, 0.30f) 		// pressed
};

// Outputs
layout (location = 0) out vec4 FragColor;


void main()
{		
	int colorIndex = ObjIndex;
	if (ObjIndex == 2) {
		if (highlighted)
			colorIndex = 3;
		if (pressed)
			colorIndex = 4;
	}
	FragColor = vec4(colors[colorIndex], 1.0f);
	if (!enabled)
		FragColor *= 0.75f;
}