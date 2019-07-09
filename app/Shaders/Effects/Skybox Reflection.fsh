/* Calculates skybox reflections. */
#version 460

layout (binding = 0) uniform sampler2DArray ColorMap;
layout (binding = 1) uniform sampler2DArray ViewNormalMap;
layout (binding = 2) uniform sampler2DArray SpecularMap;
layout (binding = 3) uniform sampler2DArray DepthMap;
layout (binding = 4) uniform samplerCube SkyMap;

layout (location = 0) in vec2 TexCoord;
layout (location = 1) flat in mat4 pMatrixInverse;
layout (location = 5) flat in mat4 vMatrixInverse;

layout (location = 0) out vec3 ReflectionColor; 

// Use PBR lighting methods
#package "lighting_pbr"


vec3 CalculateReflections(in vec3 ViewPos, in vec3 ViewNormal, in float Roughness)
{		
	vec3 ReflectDir					= (reflect(ViewPos, ViewNormal));
		 ReflectDir 				= normalize(vMatrixInverse * vec4(ReflectDir,0)).xyz;
	const float level 				= Roughness * 5.0f;
	return 							pow(textureLod(SkyMap, vec3(ReflectDir.x, -ReflectDir.y, ReflectDir.z), level).xyz, vec3(2.2f));		
}

void main()
{		
	ViewData data;
	GetFragmentData(TexCoord, data);		
	if (data.View_Depth < 1.0f) 	
		ReflectionColor				= CalculateReflections(data.View_Pos.xyz, data.View_Normal, data.Roughness);
}