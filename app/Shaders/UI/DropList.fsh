/* UI DropList Shader. */
#version 460

layout (location = 1) flat in int Index;

layout (location = 0) out vec4 FragColor;

layout (location = 3) uniform bool highlighted;
layout (location = 4) uniform bool pressed;

layout (location = 5) uniform vec4 colors[4] = { 
	vec4(1.0f), 						// Background
	vec4(0.20f, 0.80f, 0.40f, 1.0f), 	// Arrow regular
	vec4(0.40f, 0.90f, 0.60f, 1.0f), 	// Arrow highlighted
	vec4(0.10f, 0.70f, 0.30f, 1.0f) 	// Arrow pressed
};


void main()
{	
	int newIndex = Index;
	if (Index == 1) {
		if (highlighted)
			newIndex = 2;
		if (pressed)
			newIndex = 3;
	}
	FragColor = colors[newIndex];	
}