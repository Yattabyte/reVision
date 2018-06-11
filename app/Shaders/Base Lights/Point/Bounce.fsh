#version 460
#package "Lighting\gi_defines"

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

layout (location = 0) flat in int BufferIndex;

layout (binding = 0) uniform sampler2DArray ShadowPos;		// RSM position
layout (binding = 1) uniform sampler2DArray ShadowNormal; 	// RSM normals
layout (binding = 2) uniform sampler2DArray ShadowFlux;  	// RSM vpl flux

vec2 ShadowProjection( in vec3 WorldPos, out int Shadow_Spot) 
{ 	
	vec4 Position 				= buffers[indexes[BufferIndex]].lightV * vec4(WorldPos, 1.0f);
	   
	highp float Distance 		= length(Position.xyz);
	Position 				   /= Distance;         

	vec2 UVCoords;
	   
	// Front Hemisphere
	if(Position.z >= 0.0f) {
		UVCoords.x   			=  (Position.x / (1.0f + Position.z)) * 0.5f + 0.5f;
		UVCoords.y   			=  (Position.y / (1.0f + Position.z)) * 0.5f + 0.5f;
		Shadow_Spot				=  buffers[indexes[BufferIndex]].Shadow_Spot1;
	}
	// Back Hemisphere
	else {
		UVCoords.x   			= -(Position.x / (1.0f - Position.z)) * 0.5f + 0.5f;   
		UVCoords.y   			=  (Position.y / (1.0f - Position.z)) * 0.5f + 0.5f;
		Shadow_Spot				=  buffers[indexes[BufferIndex]].Shadow_Spot2;
	}	
    return 						clamp(UVCoords, vec2(0.0f), vec2(1.0f));
} 

vec3 CalcShadowPos(in vec2 TexCoord, in int ShadowSpot, in mat4 InverseVP) 
{
	const vec3 uv_array_lookup	= vec3(TexCoord, ShadowSpot);
	const float depth			= texture(ShadowPos, uv_array_lookup).r; 
	const vec4 World_Pos		= InverseVP * vec4(vec3(TexCoord, depth) * 2.0f - 1.0f, 1.0f);
	return 						World_Pos.xyz / World_Pos.w;
}

void TEST(in vec3 extents, in vec3 RHCellSize, in vec3 RHCenter, in vec2 RHUV, in mat4 InverseVP, in int ShadowSpot, in sampler2DArray ShadowPos, in sampler2DArray ShadowNormal, in sampler2DArray ShadowFlux)
{
	// Variable Initialization
    float dist, dist_min = volume_data.R_wcs, dist_max = 0.0f, dist_ave = 0.0, FF; 
	vec3 rsmColor, rsmPos, rsmNormal, color; 
    vec4 SH_dist_ave 			= vec4(0.0);
    vec4 SHr 					= vec4(0.0);  
    vec4 SHg 					= vec4(0.0); 
    vec4 SHb 					= vec4(0.0); 
	int totalSamples 			= volume_data.samples;
	
	for (int i = 0; i < totalSamples; ++i) 
    { 
		// produce a new sample location on the RSM texture
        vec3 rnd 				= 2.0f * texture(Noise, 14 * RHCenter / extents + vec3(i,0,0) / totalSamples).xyz - 1.0f; 
		vec2 uv 				= RHUV + vec2( rnd.x * volume_data.spread * cos(6.283 * rnd.y), rnd.x * volume_data.spread * sin(6.283 * rnd.y) ); 		
		vec3 uv_array_lookup	= vec3(uv, ShadowSpot);
		
		rsmPos					= CalcShadowPos(uv, ShadowSpot, InverseVP);
		rsmColor				= texture(ShadowFlux, uv_array_lookup).rgb; 
		rsmNormal 				= normalize(texture(ShadowNormal, uv_array_lookup).rgb);		
		
        // produce a new sampling location in the RH stratum
		vec3 samplePos			= RHCenter + (0.5f * rnd) * RHCellSize;		
		
		// Normalize distance to RSM sample
        dist 					= distance(samplePos, rsmPos) / volume_data.R_wcs; 
		
        // Determine the incident direction. 
        // Avoid very close samples (and numerical instability problems)
        vec3 dir 				= ( dist <= 0.007f ) ? vec3(0.0f) : normalize( samplePos - rsmPos );
		float dotprod 			= max(dot(dir, rsmNormal), 0.0f); 
		FF 						= dotprod / (0.1f + dist * dist);	
		
		color 					= rsmColor * FF;
		
		// encode radiance into SH coefs and accumulate to RH		
		vec4 shr, shg, shb; 		
		RGB2SH					(dir, color, shr, shg, shb); 
		SHr  				   += shr; 
		SHg					   += shg; 
		SHb					   += shb; 
		
		// update distance measurements
		dist_max				= ( dist > dist_max ) ? dist : dist_max; 
		dist_min				= ( dist < dist_min ) ? dist : dist_min; 
		dist_ave	   		   += dist; 
	} 

	// cast samples to float to resolve some weird compiler issue. 
	SHr						   /= float( 3.14159f * float(totalSamples) ); 
	SHg						   /= float( 3.14159f * float(totalSamples) ); 
	SHb						   /= float( 3.14159f * float(totalSamples) ); 
	dist_ave				   /= float( totalSamples ); 	
	
	GI_Out1 					= vec4( dist_min, volume_data.R_wcs - dist_max, dist_ave, 1.0f );
	GI_Out2 					= SHr; 
	GI_Out3 					= SHg; 
	GI_Out4 					= SHb; 
}

void main()
{	
	// Get current RH's world pos
	vec3 bbox_max 				= volume_data.BBox_Max.xyz;
	vec3 bbox_min 				= volume_data.BBox_Min.xyz;
	vec3 pos					= vec3(gl_FragCoord.x, gl_FragCoord.y, gl_Layer);
    vec3 extents 				= (bbox_max - bbox_min).xyz; 
	vec3 RHCellSize				= extents / (volume_data.resolution);
    vec3 RHCenter 				= bbox_min + pos * RHCellSize; 	
	
	// RH -> light space, get sampling disk center
	int Shadow_Spot;
    vec2 RHUV 					= ShadowProjection(RHCenter, Shadow_Spot);

	// Perform light bounce operation
	TEST(extents, RHCellSize, RHCenter, RHUV, Shadow_Spot, ShadowPos, ShadowNormal, ShadowFlux);
}