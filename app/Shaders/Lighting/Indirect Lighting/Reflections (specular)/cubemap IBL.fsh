#version 460
#package "Lighting\lighting_pbr"

layout (binding = 4) uniform samplerCubeArray SkyMap;
layout (location = 0) out vec3 LocalReflectionOut;  
in vec2 TexCoord;

vec3 CalculateReflections(in vec3 ViewPos, in vec3 ViewNormal, in float Roughness)
{		
	vec3 ReflectDir					= (reflect(ViewPos, ViewNormal));
		 ReflectDir 				= normalize(vMatrix_Inverse * vec4(ReflectDir,0)).xyz;
	const float level 				= Roughness * 5.0f;
	return 							texture(SkyMap, vec4(ReflectDir.x, -ReflectDir.y, -ReflectDir.z, level)).xyz;		
}

void main()
{		
	ViewData data;
	GetFragmentData(TexCoord, data);		
	if (data.View_Depth >= 1.0f) discard;
	
	LocalReflectionOut				= CalculateReflections(data.View_Pos.xyz, data.View_Normal, data.Roughness);
}