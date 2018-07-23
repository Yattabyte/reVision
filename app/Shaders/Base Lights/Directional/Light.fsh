#version 460
#define NUM_CASCADES 4
#define EPSILON 0.00001
#define saturate(value) clamp(value, 0.0f, 1.0f)
#package "Lighting\lighting_pbr"
layout (early_fragment_tests) in;

struct Light_Struct {
	mat4 lightV;
	vec4 LightColor;
	vec4 LightDirection;
	float ShadowSize_Recip;
	float LightIntensity;
	
	int Shadow_Spot;
	float CascadeEndClipSpace[NUM_CASCADES];
	mat4 LightVP[NUM_CASCADES];
	mat4 InverseLightVP[NUM_CASCADES];	
};

layout (std430, binding = 6) readonly buffer Light_Buffer {
	Light_Struct buffers[];
};

layout (location = 0) in vec2 TexCoord;
layout (location = 1) in flat uint BufferIndex;

layout (binding = 4) uniform sampler2DArray ShadowMap;
layout (location = 0) out vec3 LightingColor;

float CalcShadowFactor(in int Index, in vec4 LightSpacePos)                                                  
{     
	// Bring fragment coordinates from world space into light space, then into texture spaces
	const vec3 ProjCoords 				= LightSpacePos.xyz / LightSpacePos.w;                                  
    const vec2 UVCoords 				= 0.5f * ProjCoords.xy + 0.5f;                                                        
    const float FragmentDepth 			= 0.5f * ProjCoords.z + 0.5f;  
	
	float Factor = 0.0f;	
	for (int y = -1; y <= 1 ; ++y) 
        for (int x = -1 ; x <= 1; ++x) {
			const vec2 Offsets 			= vec2(x, y) * buffers[BufferIndex].ShadowSize_Recip;
			const vec3 FinalCoord 		= vec3( UVCoords + Offsets, buffers[BufferIndex].Shadow_Spot + Index );
			const float depth 			= texture( ShadowMap, FinalCoord ).r;
			Factor 			   		   += (depth >= FragmentDepth) ? 1.0 : 0.0;	
		}

	return Factor / 9.0f;      
} 

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
	
	// Shadow
	const float cosAngle			= saturate(1.0f - NdotL);
	const vec4 scaledNormalOffset	= vec4(data.World_Normal * (cosAngle * buffers[BufferIndex].ShadowSize_Recip), 0);
	int index 						= 0;
	for (; index < 4; ++index) 
		if (-data.View_Pos.z <= buffers[BufferIndex].CascadeEndClipSpace[index]) 
			break;			
	const vec3 LightPseudoPos		= EyePosition + (buffers[BufferIndex].LightDirection.xyz);
	const float ShadowFactor 		= CalcShadowFactor(index, buffers[BufferIndex].LightVP[index] * (data.World_Pos + scaledNormalOffset));	
	if (ShadowFactor < EPSILON)				discard; // Discard if completely in shadow
	
	// Direct Light	
	vec3 Fs;
	const vec3 Radiance 			= (buffers[BufferIndex].LightColor.xyz * buffers[BufferIndex].LightIntensity) * ShadowFactor;
	const vec3 D_Diffuse			= CalculateDiffuse( data.Albedo );
	const vec3 D_Specular			= BRDF_Specular( data.Roughness, data.Albedo, data.Metalness, data.World_Normal, -buffers[BufferIndex].LightDirection.xyz, NdotL_Clamped, NdotV_Clamped, ViewDirection, Fs);
	const vec3 D_Ratio				= (vec3(1.0f) - Fs) * (1.0f - data.Metalness);
	const vec3 Lighting 			= (D_Ratio * D_Diffuse + D_Specular) * Radiance * NdotL_Clamped; 	
	LightingColor					= Lighting;	
}
