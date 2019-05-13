/* Makes a reflection buffer more physically correct. */
#version 460 

layout (binding = 0) uniform sampler2D ColorMap;
layout (binding = 1) uniform sampler2D ViewNormalMap;
layout (binding = 2) uniform sampler2D SpecularMap;
layout (binding = 3) uniform sampler2D DepthMap;
layout (binding = 4) uniform sampler2D IndirectRadianceTexture;
layout (binding = 5) uniform sampler2D IndirectSpecularTexture;
layout (binding = 6) uniform sampler2D EnvironmentBRDF;

layout (location = 0) in vec2 TexCoord;
layout (location = 1) flat in mat4 CamPInverse;
layout (location = 5) flat in mat4 CamVInverse;
layout (location = 9) flat in vec3 CamEyePosition;

layout (location = 0) out vec3 LightingTexture;

#package "lighting_pbr" // Use PBR lighting methods


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
	if (data.View_Depth < 1.0f) {	
		const vec3 View_Direction		= normalize(CamEyePosition - data.World_Pos.xyz);		
		const float NdotV				= max(dot(data.World_Normal, View_Direction), 0.0);		
		const vec3 F0					= mix(vec3(0.03f), data.Albedo, data.Metalness);
		const vec3 Fs					= Fresnel_Schlick_Roughness(F0, NdotV, data.Roughness);
		const vec2 I_BRDF				= IntegrateBRDF(data.Roughness, data.World_Normal, NdotV);
		const vec3 I_Diffuse			= texture(IndirectRadianceTexture, TexCoord).rgb * (data.Albedo / M_PI);
		const vec3 I_Ratio				= (vec3(1.0f) - Fs) * (1.0f - data.Metalness);
		const vec3 I_Specular			= texture(IndirectSpecularTexture, TexCoord).rgb * (Fs * I_BRDF.x + I_BRDF.y);
		LightingTexture 				= (I_Ratio * I_Diffuse + I_Specular) * data.View_AO;
	}
}