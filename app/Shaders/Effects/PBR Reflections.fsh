#version 460
#package "lighting_pbr"

layout (binding = 4) uniform sampler2D ReflectionMap;
layout (location = 0) out vec3 LightingColor;
layout (location = 0) in vec2 TexCoord;


void main(void)
{   
	LightingColor						= vec3(0.0f);	
	ViewData data;
	GetFragmentData(TexCoord, data);	
	if (data.View_Depth >= 1.0f) 		discard;	
	const vec3 Reflection				= texture(ReflectionMap, TexCoord).rgb;		
	LightingColor 						= Reflection;
}