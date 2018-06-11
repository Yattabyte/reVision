#version 460
#define saturate(value) clamp(value, 0.0f, 1.0f)
#package "Lighting\lighting_pbr"

struct Light_Struct {
	vec4 LightColor;
	vec4 LightDirection;
	float LightIntensity;
};

layout (std430, binding = 6) readonly buffer Light_Buffer {
	Light_Struct buffers[];
};

in vec2 TexCoord;
in flat uint BufferIndex;

layout (location = 0) out vec3 LightingColor;

void main()
{			
	// Initialize first variables
	LightingColor 					= vec3(0);
	ViewData data;
	GetFragmentData(TexCoord, data);	
   	if (data.View_Depth >= 1.0f)			discard; // Discard background fragments
	
	const vec3 ViewDirection		= normalize(EyePosition - data.World_Pos.xyz);
	const float NdotV 				= dot(data.World_Normal, ViewDirection);
	const float NdotL 		 		= dot(normalize(-buffers[BufferIndex].LightDirection.xyz), data.World_Normal);
	const float NdotL_Clamped		= max(NdotL, 0.0);
	const float NdotV_Clamped		= max(NdotV, 0.0);
	if (NdotL < 0.f && abs(NdotV) < 0.f)	discard; // Discard if light will be zero anyway
	
	// Direct Light	
	vec3 Fs;
	const vec3 Radiance 			= (buffers[BufferIndex].LightColor.xyz * buffers[BufferIndex].LightIntensity);
	const vec3 D_Diffuse			= CalculateDiffuse( data.Albedo );
	const vec3 D_Specular			= BRDF_Specular( data.Roughness, data.Albedo, data.Metalness, data.World_Normal, -buffers[BufferIndex].LightDirection.xyz, NdotL_Clamped, NdotV_Clamped, ViewDirection, Fs);
	const vec3 D_Ratio				= (vec3(1.0f) - Fs) * (1.0f - data.Metalness);
	const vec3 Lighting 			= (D_Ratio * D_Diffuse + D_Specular) * Radiance * NdotL_Clamped; 	
	LightingColor					= Lighting;	
}
