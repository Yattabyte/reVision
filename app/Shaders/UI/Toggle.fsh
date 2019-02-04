/* UI Toggle Shader. */
#version 460

// Inputs
layout (location = 0) in vec2 TexCoord;
layout (location = 1) flat in int ObjIndex;

// Uniforms
layout (location = 2) uniform bool enabled;
layout (location = 3) uniform bool highlighted;
layout (location = 4) uniform bool pressed;
layout (location = 5) uniform bool toggled_on;
layout (location = 6) uniform vec3 colors[8] = { 
	vec3(0.40f), 				// Background - Off
	vec3(0.20f, 0.30f, 0.40f), 	// Knob - Off
	vec3(0.20f, 0.40f, 0.50f), 	// Highlight - Off
	vec3(0.10f, 0.20f, 0.30f), 	// Pressed - Off
	
	vec3(0.20f, 0.30f, 0.40f), 	// Background - On
	vec3(0.20f, 0.80f, 0.40f),	// Knob - On
	vec3(0.40f, 0.90f, 0.60f), 	// Highlight - On
	vec3(0.10f, 0.70f, 0.30f) 	// Pressed - On	
};

// Outputs
layout (location = 0) out vec4 FragColor;


void main()
{	
	int colorIndex = ObjIndex;
	if (ObjIndex > 0) {
		colorIndex = 1;
		if (highlighted)
			colorIndex = 2;
		if (pressed)
			colorIndex = 3;	
	}
	if (toggled_on)
		colorIndex += 4;	
	FragColor = vec4(colors[colorIndex], 1.0f);	
	if (!enabled)
		FragColor *= 0.75f;
}