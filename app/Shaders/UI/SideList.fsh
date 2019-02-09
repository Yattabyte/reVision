/* UI DropList Shader. */
#version 460

// Inputs
layout (location = 1) flat in int ObjIndex;

// Uniforms
layout (location = 1) uniform bool enabled;
layout (location = 2) uniform bool lhighlighted;
layout (location = 3) uniform bool rhighlighted;
layout (location = 4) uniform bool lpressed;
layout (location = 5) uniform bool rpressed;
layout (location = 6) uniform vec3 colors[4] = { 
	vec3(1.0f), 				// Background
	vec3(0.20f, 0.80f, 0.40f), 	// Arrow regular
	vec3(0.40f, 0.90f, 0.60f), 	// Arrow highlighted
	vec3(0.10f, 0.70f, 0.30f) 	// Arrow pressed
};

// Outputs
layout (location = 0) out vec4 FragColor;


void main()
{	
	int newIndex = ObjIndex;
	if (ObjIndex == 1) {
		newIndex = 1;
		if (lhighlighted)
			newIndex = 2;
		if (lpressed)
			newIndex = 3;
	}
	else if (ObjIndex == 2) {
		newIndex = 1;
		if (rhighlighted)
			newIndex = 2;
		if (rpressed)
			newIndex = 3;
	}
	FragColor = vec4(colors[newIndex], 1.0f);
	if (!enabled)
		FragColor *= 0.75f;
}