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
layout (location = 8) uniform vec4 color =  vec4(0.75f);

// Outputs
layout (location = 0) out vec4 FragColor;


void main()
{			
	FragColor = color;		
	if (ObjIndex == 0) {	
		if (lhighlighted)
			FragColor *= 1.5f;
		if (lpressed)
			FragColor *= 0.5f;
		if (!lEnabled)
			FragColor *= 0.25f;
	}
	else if (ObjIndex == 1) {
		if (rhighlighted)
			FragColor *= 1.5f;
		if (rpressed)
			FragColor *= 0.5f;
		if (!rEnabled)
			FragColor *= 0.25f;
	}
	if (!enabled)
		FragColor *= 0.5f;
}