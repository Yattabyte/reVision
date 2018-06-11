#version 460

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

layout (location = 0) out vec4 GI_Out1; 
layout (location = 1) out vec4 GI_Out2; 
layout (location = 2) out vec4 GI_Out3; 
layout (location = 3) out vec4 GI_Out4;

layout (binding = 4) uniform sampler3D Noise; // A pre-computed 3D noise texture (32X32X32). Value range (r,g,b): [0,1] 
layout (binding = 5) uniform sampler3D VolumeMap1;
layout (binding = 6) uniform sampler3D VolumeMap2;
layout (binding = 7) uniform sampler3D VolumeMap3;
layout (binding = 8) uniform sampler3D VolumeMap4;


vec4 SHBasis (const in vec3 dir) 
{ 
    float L00  					= 0.282095; 
    float L1_1 					= 0.488603 * dir.y; 
    float L10  					= 0.488603 * dir.z; 
    float L11  					= 0.488603 * dir.x; 
    return vec4 				(L11, L1_1, L10, L00); 
}

void RGB2SH (in vec3 dir, in vec3 L, out vec4 sh_r, out vec4 sh_g, out vec4 sh_b) 
{ 
    vec4 sh 					= SHBasis (dir); 
    sh_r 						= L.r * sh; 
    sh_g 						= L.g * sh; 
    sh_b 						= L.b * sh; 
}

vec3 SH2RGB (in vec4 sh_r, in vec4 sh_g, in vec4 sh_b, in vec3 dir) 
{ 
	vec4 Y 						= vec4(1.023326 * dir.x, 1.023326 * dir.y, 1.023326 * dir.z, 0.886226);
    return 						vec3(dot(Y, sh_r), dot(Y, sh_g), dot(Y, sh_b));
}

void BounceFromBounce(in vec3 extents, in vec3 uvw)
{
	// Variable Initialization
    float dist, FF = 1.0f, scale = 1.0f, uvdist = volume_data.R_wcs / length(extents);  // approx. max. distance in texture coords.
    float mean_res 				= length(vec3(volume_data.resolution)); 
    vec3 GI						= vec3(0.0f);
    vec4 SHr					= vec4(0.0f); 
    vec4 SHg					= vec4(0.0f); 
    vec4 SHb					= vec4(0.0f); 
	int totalSamples 			= volume_data.samples;
	
	for (int i = 0; i < totalSamples; ++i) 
    { 
        // generate a random vector that depends on position and iteration
        // Can be improved to reduce texture lookups by adding the iteration permutations
        // as part of a larger noise texture (concatenated 3D noise tables, indexed by iteration)
		vec3 rnd 				= ( texture(Noise, 5 * uvw.xyz + vec3(0,i,0) / totalSamples).xyz + 
									texture(Noise, 7 * uvw.yzx + vec3(i,0,0) / totalSamples).zxy ) / 2.0f; 
		rnd 					= 2.0f * texture(Noise, 4 * rnd.yzx + vec3(0,0,i) / totalSamples).xzy - 1.0f; 

		// Determine a sampling direction for the i-th integral sample
		vec3 dir 				= normalize(rnd); 

		// Determine the distance of the RH sample. It should be sufficiently far from the 
		// current RH in order to avoid self-illumination
		dist 					= 0.5f / mean_res + rnd.x * rnd.x; 
			
		// Determine the texture coords of the RH sample and read data from the RH buffer
		vec3 uvw_new   			= volume_data.resolution*dir*dist*uvdist/mean_res + uvw; 
		vec4 rh_shr    			= texture(VolumeMap2, uvw_new); 
		vec4 rh_shg    			= texture(VolumeMap3, uvw_new); 
		vec4 rh_shb    			= texture(VolumeMap4, uvw_new); 
		vec2 distances 			= texture(VolumeMap1, uvw_new).xy; 

		// Weight the sample
		float V 				= 1.0f - clamp((dist - distances.x) / (distances.y - distances.x), 0.0, 1.0f); 
		float weight 			= clamp(1.0f - distances.x, 0.0, 1.0f); 
		GI 						= weight * V * SH2RGB(rh_shr, rh_shg, rh_shb, -dir); 
		scale 				   += weight * V; 
	
        // encode radiance sample as contributing to the incident direction
        RGB2SH					(dir, GI, rh_shr, rh_shg, rh_shb); 
		SHr					   += FF * rh_shr; 
		SHg					   += FF * rh_shg; 
		SHb					   += FF * rh_shb; 
    }  

    scale 						= 12.567 / scale; 

    // Accumulate the result. Since we do ping pong rendering, the current voxel RH data 
    // must also be read. Distance data just pass through.
    GI_Out1 					= texture(VolumeMap1, uvw); 
    GI_Out2 					= SHr * scale + texture(VolumeMap2, uvw); 
    GI_Out3 					= SHg * scale + texture(VolumeMap3, uvw); 
    GI_Out4 					= SHb * scale + texture(VolumeMap4, uvw); 
}

void main(void) 
{ 	
    // Find the RH position in WCS and texture space:	
	vec3 bbox_max 				= volume_data.BBox_Max.xyz;
	vec3 bbox_min 				= volume_data.BBox_Min.xyz;
	vec3 g 						= vec3(gl_FragCoord.x, gl_FragCoord.y, gl_Layer);
    vec3 extents 				= (bbox_max - bbox_min); 
    vec3 stratum 				= extents / volume_data.resolution; 
    vec3 pos 					= bbox_min + vec3(g + 0.5f) * stratum; 
    vec3 uvw 					= (pos - bbox_min) / extents; 
	
    BounceFromBounce(extents, uvw);
}