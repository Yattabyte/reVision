/* UI DropList Shader. */
#version 460

// Inputs
layout (location = 1) flat in int ObjIndex;

// Uniforms
layout (location = 1) uniform bool enabled;
layout (location = 2) uniform bool lEnabled;
layout (location = 3) uniform bool rEnabled;
layout (location = 4) uniform bool lhighlighted;
layout (location = 5) uniform bool rhighlighted;
layout (location = 6) uniform bool lpressed;
layout (location = 7) uniform bool rpressed;
layout (location = 8) uniform vec3 colors[2] = { 
	vec3(1.0f), 				// Background
	vec3(0.20f, 0.80f, 0.40f) 	// Arrow
};

// Outputs
layout (location = 0) out vec4 FragColor;


void main()
{			
	if (ObjIndex == 0)
		FragColor = vec4(colors[0], 1.0f);
	else if (ObjIndex == 1) {
		FragColor = vec4(colors[1], 1.0f);			
		if (lhighlighted)
			FragColor.xyz *= 1.25f;
		if (lpressed)
			FragColor.xyz *= 0.75f;
		if (!lEnabled)
			FragColor.xyz *= 0.50f;
	}
	else if (ObjIndex == 2) {
		FragColor = vec4(colors[1], 1.0f);			
		if (rhighlighted)
			FragColor.xyz *= 1.25f;
		if (rpressed)
			FragColor.xyz *= 0.75f;
		if (!rEnabled)
			FragColor.xyz *= 0.50f;
	}
	if (!enabled)
		FragColor.xyz *= 0.75f;
}