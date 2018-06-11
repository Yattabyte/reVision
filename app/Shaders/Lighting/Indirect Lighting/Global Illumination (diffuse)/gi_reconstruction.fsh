#version 460
#package "Lighting\lighting_pbr"

struct Volume_Data 
{
	vec4 BBox_Max;
	vec4 BBox_Min;
	int samples;
	int resolution;
	float spread;
	float R_wcs;
	float factor;
};

layout (std430, binding = 7) readonly buffer GI_Volume_Attribs
{			
	Volume_Data volume_data;
};

layout (binding = 5) uniform sampler3D Noise;
layout (binding = 6) uniform sampler3D VolumeMap1;
layout (binding = 7) uniform sampler3D VolumeMap2;
layout (binding = 8) uniform sampler3D VolumeMap3;
layout (binding = 9) uniform sampler3D VolumeMap4;

layout (location = 0) in vec2 TexCoord;

layout (location = 0) out vec3 LightingColor;

vec3 Fresnel_Schlick_Roughness(in vec3 f0, in float AdotB, in float roughness)
{
    return 						f0 + (max(vec3(1.0 - roughness), f0) - f0) * pow(1.0 - AdotB, 5.0);
}   

vec3 SH2RGB (in vec4 sh_r, in vec4 sh_g, in vec4 sh_b, in vec3 dir) 
{ 
    vec4 Y 						= vec4(1.023326 * dir.x, 1.023326 * dir.y, 1.023326 * dir.z, 0.886226);
    return 						vec3 (dot(Y,sh_r), dot(Y,sh_g), dot(Y,sh_b));
}

vec3 CalculateGI(in vec3 WorldPos, in vec3 Normal, in vec3 extents, in vec3 uvw)
{	
    // Sample the RH volume at 4 locations, one directly above the shaded point,
    // three on a ring 80degs away from the normal direction.	
	// You can introduce a random rotation of the samples around the normal
    vec3 rnd 					= vec3(0.0f);
    //vec3 rnd 					= texture(Noise, 25 * uvw).xyz - vec3(0.5,0.5,0.5); 
    vec3 D[4]; 
		 D[0] 					= vec3(1.0, 0.0, 0.0); 		 
    
    for (int i = 1; i < 4; ++i)
		D[i] = normalize( vec3(0.1, 0.8 * cos( (rnd.x * 1.5 + i) * 6.2832 / 3.0 ), 0.8 * sin( (rnd.x * 1.5 + i) * 6.2832 / 3.0) ) );    

	float extents_length		= length(extents / volume_data.resolution);
    vec3 v_rand 				= vec3(0.5, 0.5, 0.5); 
    vec3 tangent 				= normalize(cross(Normal, v_rand)); 
    vec3 bitangent 				= cross(Normal, tangent); 
	vec3 GI						= vec3(0.0f);	
	float denom 				= 0.05;
    for (int i = 0; i < 4; ++i) { 
		vec3 SampleDir 			= Normal * D[i].x + tangent * D[i].y + bitangent * D[i].z;
        vec3 uvw_new 			= 0.5 * Normal / volume_data.resolution + SampleDir / volume_data.resolution + uvw;

		vec4 rh_shr    			= texture(VolumeMap2, uvw_new); 
		vec4 rh_shg    			= texture(VolumeMap3, uvw_new); 
		vec4 rh_shb    			= texture(VolumeMap4, uvw_new); 
		
		vec3 rh_pos    			= volume_data.BBox_Min.xyz + extents * uvw_new; 
		float dist 				= length(rh_pos - WorldPos) / extents_length; 
		
		float contribution		= dist > 0.005 ? 1.0 : 0.0;
		GI					   += contribution * SH2RGB(rh_shr, rh_shg, rh_shb, -Normal);
		denom				   += contribution;
    } 
	
    GI *= volume_data.factor / denom; 
	return max( GI, vec3(0.0f) );
}

void main()
{	
	// Initialize first variables
	LightingColor 					= vec3(0);
	ViewData data;
	GetFragmentData(TexCoord, data);
	
	// Discard background fragments
    if (data.View_Depth >= 1.0f)  	discard;         	
	
	const vec3 ViewDirection		= normalize(EyePosition - data.World_Pos.xyz);
	const float NdotV 				= max(dot(data.World_Normal, ViewDirection), 0.0);	
	
	// We want it to clamp to edge and repeat the edge texture, as we aren't using any other cascades
	const vec3 extents 				= (volume_data.BBox_Max.xyz - volume_data.BBox_Min.xyz); 
	const vec3 uvw 					= (data.World_Pos.xyz - volume_data.BBox_Min.xyz) / extents;  
	
	// Ambient Diffuse (Indirect)
	const vec3 GI_Irradiance 		= CalculateGI(data.World_Pos.xyz, data.World_Normal, extents, uvw);	
	const vec3 F0					= mix(vec3(0.03f), data.Albedo, data.Metalness);
    const vec3 Fs 					= Fresnel_Schlick_Roughness(F0, NdotV, data.Roughness);	
	const vec3 I_Ratio				= (vec3(1.0f) - Fs) * (1.0f - data.Metalness);		 
	const vec3 I_Diffuse 			= (data.Albedo / 3.14159f);
	LightingColor					= ((I_Ratio * I_Diffuse) * GI_Irradiance) * data.View_AO; 
}