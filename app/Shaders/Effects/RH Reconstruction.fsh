#version 460
#pragma optionNV(fastmath on)
#pragma optionNV(ifcvt none)
#pragma optionNV(inline all)
#pragma optionNV(strict on)
#pragma optionNV(unroll all)
#define EPSILON 0.00001

layout (binding = 0) uniform sampler2DArray ColorMap;
layout (binding = 1) uniform sampler2DArray ViewNormalMap;
layout (binding = 2) uniform sampler2DArray SpecularMap;
layout (binding = 3) uniform sampler2DArray DepthMap;
layout (binding = 4) uniform sampler3D VolumeMap1;
layout (binding = 5) uniform sampler3D VolumeMap2;
layout (binding = 6) uniform sampler3D VolumeMap3;
layout (binding = 7) uniform sampler3D VolumeMap4;

layout (location = 1) uniform vec3 BBox_Max = vec3(1);
layout (location = 2) uniform vec3 BBox_Min = vec3(-1);
layout (location = 3) uniform float resolution = 16.0f;
layout (location = 4) uniform float factor = 1.0f;

layout (location = 0) in vec2 TexCoord;
layout (location = 1) flat in mat4 CamPInverse;
layout (location = 5) flat in mat4 CamVInverse;

layout (location = 0) out vec3 LightingColor;

// Use PBR lighting methods
#package "lighting_pbr"

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

	float extents_length		= length(extents / resolution);
    vec3 v_rand 				= vec3(0.5f); 
    vec3 tangent 				= normalize(cross(Normal, v_rand)); 
    vec3 bitangent 				= cross(Normal, tangent); 
	vec3 GI						= vec3(0.0f);	
	float denom 				= EPSILON;
    for (int i = 0; i < 4; ++i) { 
		vec3 SampleDir 			= Normal * D[i].x + tangent * D[i].y + bitangent * D[i].z;
        vec3 uvw_new 			= 0.5f * Normal / resolution + SampleDir / resolution + uvw;

		vec4 rh_shr    			= texture(VolumeMap2, uvw_new); 
		vec4 rh_shg    			= texture(VolumeMap3, uvw_new); 
		vec4 rh_shb    			= texture(VolumeMap4, uvw_new); 
		
		vec3 rh_pos    			= BBox_Min.xyz + extents * uvw_new; 
		float dist 				= length(rh_pos - WorldPos) / extents_length; 
		
		float contribution		= dist > 0.005f ? 1.0f : 0.0f;
		GI					   += contribution * SH2RGB(rh_shr, rh_shg, rh_shb, -Normal);
		denom				   += contribution;
    } 
	
    GI *= factor / denom; 
	return max( GI, vec3(0.0f) );
}

void main()
{	
	// Initialize first variables
	ViewData data;
	GetFragmentData(TexCoord, data);
    if (data.View_Depth >= 1.0f)  	discard;         	
	
	// We want it to clamp to edge and repeat the edge texture, as we aren't using any other cascades
	const vec3 extents 				= (BBox_Max.xyz - BBox_Min.xyz); 
	const vec3 uvw 					= (data.World_Pos.xyz - BBox_Min.xyz) / extents; 
	LightingColor 					= CalculateGI(data.World_Pos.xyz, data.World_Normal, extents, uvw);	
}