#version 460
#define EPSILON 0.00001
#define saturate(value) clamp(value, 0.0f, 1.0f)
#package "Lighting\lighting_pbr"
layout (early_fragment_tests) in;

struct Light_Struct {
	mat4 mMatrix;
	vec4 LightColor;
	vec4 LightPosition;
	vec4 LightVector;
	float LightIntensity;
	float LightRadius;
	float LightCutoff;
};

layout (std430, binding = 3) readonly buffer Visibility_Buffer {
	uint indexes[];
};

layout (std430, binding = 6) readonly buffer Light_Buffer {
	Light_Struct buffers[];
};

layout (location = 0) flat in uint BufferIndex;
layout (location = 0) uniform bool UseStencil = true;
layout (location = 0) out vec3 LightingColor;         

vec2 CalcTexCoord()
{
    return			 				gl_FragCoord.xy / CameraDimensions;
}

void main(void)
{		
	// Initialize variables
	LightingColor 					= vec3(0);
	if (UseStencil) 				return; // Strictly for the stenciling pass so we don't have to change shaders		
	ViewData data;
	GetFragmentData(CalcTexCoord(), data);
    if (data.View_Depth >= 1.0f) 	discard; // Discard background fragments
	
	// Spot Angle Factor
	const vec3 LightToPixel 		= normalize(data.World_Pos.xyz - buffers[indexes[BufferIndex]].LightPosition.xyz);                             
	const float SpotFactor 			= dot(LightToPixel, buffers[indexes[BufferIndex]].LightVector.xyz); 			
    if (SpotFactor < buffers[indexes[BufferIndex]].LightCutoff)	discard;	// Discard if light falls outside of FOV
	
	const vec3 LightPosDirection 	= normalize(buffers[indexes[BufferIndex]].LightPosition.xyz - data.World_Pos.xyz);  
	const vec3 DeltaView 			= EyePosition - data.World_Pos.xyz;  
	const float ViewDistance 		= length(DeltaView);
	const vec3 ViewDirection		= DeltaView / ViewDistance;
	const float NdotV 				= dot(data.World_Normal, ViewDirection);
	const float NdotL 		 		= dot(normalize(LightPosDirection), data.World_Normal);
	const float NdotL_Clamped		= max(NdotL, 0.0);
	const float NdotV_Clamped		= max(NdotV, 0.0);
	if (NdotL < 0.f && abs(NdotV) < 0.f)	discard; // Discard if light will be zero anyway
	
	// Attenuation	
	const float Distance 			= length(buffers[indexes[BufferIndex]].LightPosition.xyz - data.World_Pos.xyz);
	const float range 				= (1.0f / (buffers[indexes[BufferIndex]].LightRadius * buffers[indexes[BufferIndex]].LightRadius));
	const float Attenuation 		= 1.0f - (Distance * Distance) * (range * range); 	
	if (Attenuation < 0.0f) 		discard;	// Discard if outside of radius	
	
	// Direct Light
	vec3 Fs;
	const vec3 Radiance 			= ( (buffers[indexes[BufferIndex]].LightColor.xyz * buffers[indexes[BufferIndex]].LightIntensity) * Attenuation ) * (1.0f - (1.0f - SpotFactor) * 1.0f/(1.0f - buffers[indexes[BufferIndex]].LightCutoff));
	const vec3 D_Diffuse			= CalculateDiffuse( data.Albedo );
	const vec3 D_Specular			= BRDF_Specular( data.Roughness, data.Albedo, data.Metalness, data.World_Normal, LightPosDirection, NdotL_Clamped, NdotV, ViewDirection, Fs);
	const vec3 D_Ratio				= (vec3(1.0f) - Fs) * (1.0f - data.Metalness); 
	const vec3 Lighting 			= (D_Ratio * D_Diffuse + D_Specular) * Radiance * NdotL_Clamped; 
	
	LightingColor					= Lighting;	
}
