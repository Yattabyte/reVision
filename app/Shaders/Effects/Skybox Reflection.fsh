#version 460
#package "lighting_pbr"

layout (binding = 4) uniform samplerCube SkyMap;
layout (binding = 5) uniform sampler2D EnvironmentBRDF;
layout (location = 0) out vec3 LocalReflectionOut;  
layout (location = 0) in vec2 TexCoord;

vec3 CalculateReflections(in vec3 ViewPos, in vec3 ViewNormal, in float Roughness)
{		
	vec3 ReflectDir					= (reflect(ViewPos, ViewNormal));
		 ReflectDir 				= normalize(cameraBuffer.vMatrix_Inverse * vec4(ReflectDir,0)).xyz;
	const float level 				= Roughness * 5.0f;
	return 							pow(texture(SkyMap, vec3(ReflectDir.x, -ReflectDir.y, ReflectDir.z)).xyz, vec3(2.2f));		
}

vec3 Fresnel_Schlick_Roughness(vec3 f0, float AdotB, float roughness)
{
    return 								f0 + (max(vec3(1.0 - roughness), f0) - f0) * pow(1.0 - AdotB, 5.0);
}   

vec2 IntegrateBRDF( in float Roughness, in vec3 Normal, in float NoV )
{
	return 								texture(EnvironmentBRDF, vec2(NoV, Roughness)).xy;
}

void main()
{		
	ViewData data;
	GetFragmentData(TexCoord, data);		
	if (data.View_Depth >= 1.0f) discard;
	
	const vec3 Reflection				= CalculateReflections(data.View_Pos.xyz, data.View_Normal, data.Roughness);	
	const vec3 View_Direction			= normalize(cameraBuffer.EyePosition - data.World_Pos.xyz);		
	const float NdotV					= max(dot(data.World_Normal, View_Direction), 0.0);		
	const vec3 F0						= mix(vec3(0.03f), data.Albedo, data.Metalness);
	const vec3 Fs						= Fresnel_Schlick_Roughness(F0, NdotV, data.Roughness);
	const vec2 I_BRDF					= IntegrateBRDF(data.Roughness, data.World_Normal, NdotV);
	const vec3 I_Diffuse				= (data.Albedo / M_PI);
	const vec3 I_Ratio					= (vec3(1.0f) - Fs) * (1.0f - data.Metalness);
	const vec3 I_Specular				= Reflection * (Fs * I_BRDF.x + I_BRDF.y);
	LocalReflectionOut 					= (I_Ratio * I_Diffuse + I_Specular) * data.View_AO;
}