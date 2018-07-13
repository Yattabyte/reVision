#version 460
#define EPSILON 0.00001
#define saturate(value) clamp(value, 0.0f, 1.0f)
#package "Lighting\lighting_pbr"
layout (early_fragment_tests) in;

struct Light_Struct {
	mat4 mMatrix;
	mat4 lightV; 
	mat4 lightPV[6];
	mat4 inversePV[6];
	vec4 LightColor;
	vec4 LightPosition;
	float ShadowSize_Recip;
	float LightIntensity;
	float LightRadius;
	int Shadow_Spot;
};

layout (std430, binding = 3) readonly buffer Visibility_Buffer {
	uint indexes[];
};

layout (std430, binding = 6) readonly buffer Light_Buffer {
	Light_Struct buffers[];
};

layout (location = 0) in flat uint BufferIndex;

layout (binding = 4) uniform samplerCubeArray ShadowMap;
layout (binding = 5) uniform samplerCubeArray ShadowMap_Static;
layout (location = 0) uniform bool UseStencil = true;
layout (location = 0) out vec3 LightingColor; 

vec2 CalcTexCoord()
{
    return			 				gl_FragCoord.xy / CameraDimensions;
}

float CalcRandom(in vec4 seed) 
{
	const float dot_product 		= dot(seed, vec4(12.9898,78.233,45.164,94.673));
    return 							fract(sin(dot_product) * 43758.5453);
}

float CalcShadowFactor(in vec3 LightDirection, in float ViewDistance, in float bias)                                                  
{
	const vec3 sampleOffsetDirections[20] = vec3[] (
		vec3( 1,  1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1,  1,  1), 
		vec3( 1,  1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1,  1, -1),
		vec3( 1,  1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1,  1,  0),
		vec3( 1,  0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1,  0, -1),
		vec3( 0,  1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0,  1, -1)
	);
	const float FragmentDepth 		= length(LightDirection);
	const float FarPlane 			= (buffers[indexes[BufferIndex]].LightRadius * buffers[indexes[BufferIndex]].LightRadius);
	const float diskRadius 			= (ViewDistance / FarPlane) / 2;
	
	float Factor = 0.0f;	
	for (int x = 0; x < 20; ++x) {	
		const vec4 FinalCoord		= vec4(LightDirection + sampleOffsetDirections[x] * diskRadius, buffers[indexes[BufferIndex]].Shadow_Spot);
		const float depth 			= min(	texture(ShadowMap, FinalCoord).r, 
											texture(ShadowMap_Static, FinalCoord).r
										) * FarPlane;
		Factor 			   	 		+= (depth >= FragmentDepth - bias - EPSILON) ? 1.0 : 0.0;	
	}
	return 							Factor / 20.0f;
}   

void main(void)
{		
	// Initialize variables
	LightingColor 					= vec3(0);
	if (UseStencil) 				return; // Strictly for the stenciling pass so we don't have to change shaders	
	ViewData data;
	GetFragmentData(CalcTexCoord(), data);
    if (data.View_Depth >= 1.0f) 	discard; // Discard background fragments
	
	const vec3 LightDirection 		= (buffers[indexes[BufferIndex]].LightPosition.xyz - data.World_Pos.xyz);
	const vec3 DeltaView 			= EyePosition - data.World_Pos.xyz;  
	const float ViewDistance 		= length(DeltaView);
	const vec3 ViewDirection		= DeltaView / ViewDistance;
	const float NdotV 				= dot(data.World_Normal, ViewDirection);
	const float NdotL 		 		= dot(normalize(LightDirection.xyz), data.World_Normal);
	const float NdotL_Clamped		= max(NdotL, 0.0);
	const float NdotV_Clamped		= max(NdotV, 0.0);
	if (NdotL < 0.f && NdotV < 0.f)	discard; // Discard if light will be zero anyway
	
	// Attenuation	
	const float Distance 			= length(buffers[indexes[BufferIndex]].LightPosition.xyz - data.World_Pos.xyz);
	const float range 				= (1.0f / (buffers[indexes[BufferIndex]].LightRadius * buffers[indexes[BufferIndex]].LightRadius));
	const float Attenuation 		= 1.0f - (Distance * Distance) * (range * range);
	if (Attenuation < 0.0f) 		discard;// Discard if outside of radius
	
	// Shadow
	const float cosAngle			= saturate(1.0f - NdotL);
	const float bias 				= clamp(0.005 * tan(acos(NdotL)), 0.0, 0.01);
	const vec3 scaledNormalOffset	= data.World_Normal * (cosAngle * buffers[indexes[BufferIndex]].ShadowSize_Recip);
	const float ShadowFactor 		= CalcShadowFactor(-(LightDirection + scaledNormalOffset), ViewDistance, bias);
	if (ShadowFactor < EPSILON) 	discard; // Discard if completely in shadow
	
	// Direct Light	
	vec3 Fs;
	const vec3 Radiance 			= ( (buffers[indexes[BufferIndex]].LightColor.xyz * buffers[indexes[BufferIndex]].LightIntensity) * Attenuation ) * ShadowFactor;
	const vec3 D_Diffuse			= CalculateDiffuse( data.Albedo );
	const vec3 D_Specular			= BRDF_Specular( data.Roughness, data.Albedo, data.Metalness, data.World_Normal, LightDirection, NdotL_Clamped, NdotV_Clamped, ViewDirection, Fs);
	const vec3 D_Ratio				= (vec3(1.0f) - Fs) * (1.0f - data.Metalness); 
	LightingColor 					= (D_Ratio * D_Diffuse + D_Specular) * Radiance * NdotL_Clamped; 
}
