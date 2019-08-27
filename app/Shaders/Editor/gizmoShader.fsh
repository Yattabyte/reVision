/* Gizmo shader. */
#version 460

layout (location = 0) in vec2 UV;
layout (location = 1) flat in uint axisID;

layout (location = 0) out vec4 fragColor;

layout (location = 4) uniform uint SelectedAxes = 0;
layout (location = 5) uniform vec3 ColorScheme[6] = {
	vec3(1,0,0),
	vec3(0,1,0),
	vec3(0,0,1),
	vec3(1,1,0),
	vec3(1,0,1),
	vec3(0,1,1)
};

const uint AXES[6] = {
	1u,	2u,	4u,
	3u,	5u,	6u
};

void main()
{	
	if ((SelectedAxes == AXES[axisID]))
		fragColor = vec4(1, 0.8, 0, 1);	
	else 
		fragColor = vec4(ColorScheme[axisID], 0.75f);
}

