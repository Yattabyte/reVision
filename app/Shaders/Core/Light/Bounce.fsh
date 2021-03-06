/* Bounce lighting. */
#version 460
#pragma optionNV(unroll all)
#define MAX_PERSPECTIVE_ARRAY 6

struct Light_Struct {
	mat4 LightVP[MAX_PERSPECTIVE_ARRAY];
	vec4 LightColor;
	vec4 LightPosition;
	float CascadeEndClipSpace[MAX_PERSPECTIVE_ARRAY];
	float LightIntensity;
	int Shadow_Spot;
	int Light_Type;
};
layout (std430, binding = 8) readonly buffer Light_Buffer {
	Light_Struct lightBuffers[];
};

layout (location = 1) uniform vec3 BBox_Max = vec3(1);
layout (location = 2) uniform vec3 BBox_Min = vec3(-1);
layout (location = 3) uniform int samples = 16;
layout (location = 4) uniform float resolution = 16.0f;
layout (location = 5) uniform float spread = 0.05f;
layout (location = 6) uniform float R_wcs = 0.0f;

layout (location = 0) flat in mat4 vMatrix;
layout (location = 4) flat in mat4 CamPVMatrix;
layout (location = 8) flat in int Shadow_Spot;
layout (location = 9) flat in vec3 ColorModifier;
layout (location = 10) flat in int lightIndex;
layout (location = 11) flat in int lightType;

layout (location = 0) out vec4 GI_Out1; 
layout (location = 1) out vec4 GI_Out2; 
layout (location = 2) out vec4 GI_Out3; 
layout (location = 3) out vec4 GI_Out4; 

layout (binding = 0) uniform sampler2DArray ShadowNormal; 	// RSM normals
layout (binding = 1) uniform sampler2DArray ShadowFlux;  	// RSM vpl flux
layout (binding = 2) uniform sampler2DArray ShadowPos;		// RSM position
layout (binding = 3) uniform sampler2DArray CameraDepth;  	// Camera depth buffer
layout (binding = 4) uniform sampler3D Noise;       		// A pre-computed 3D noise texture (32X32X32). Value range (r,g,b): [0,1] 
// #define DEPTH_OCCL    // if defined, depth-based RSM sample occlusion is enabled.


vec4 SHBasis(const in vec3 dir) 
{ 
    float L00  					= 0.282095; 
    float L1_1 					= 0.488603 * dir.y; 
    float L10  					= 0.488603 * dir.z; 
    float L11  					= 0.488603 * dir.x; 
    return vec4 				(L11, L1_1, L10, L00); 
}

void RGB2SH(in vec3 dir, in vec3 L, out vec4 sh_r, out vec4 sh_g, out vec4 sh_b) 
{ 
    vec4 sh 					= SHBasis (dir); 
    sh_r 						= L.r * sh; 
    sh_g 						= L.g * sh; 
    sh_b 						= L.b * sh; 
}

vec3 PointWCS2CSS(in vec3 p) 
{ 
    vec4 p_css = CamPVMatrix * vec4(p,1); 
    return p_css.xyz/p_css.w; 
} 

void CalcDirectionalShadow(in vec3 RHCenter, in float View_Depth, out vec2 RHUV, out mat4 lightVP, out int finalShadowSpot) 
{
	int index 					= 0;
	for (; index < 4; ++index)
		if (View_Depth <= lightBuffers[lightIndex].CascadeEndClipSpace[index])
			break;
	const vec4 LightSpacePos 	= lightBuffers[lightIndex].LightVP[index] * vec4(RHCenter, 1);
	const vec3 ProjCoords 		= LightSpacePos.xyz / LightSpacePos.w;                                  
    const vec2 UVCoords 		= 0.5 * ProjCoords.xy + 0.5;
    RHUV 						= clamp(UVCoords, vec2(0.0f), vec2(1.0f));
	lightVP 					= lightBuffers[lightIndex].LightVP[index];
	finalShadowSpot				= Shadow_Spot + index;
} 

void CalcPointShadow(in vec3 RHCenter, out vec2 RHUV, out mat4 lightVP, out int finalShadowSpot) 
{
	const vec3 v 				= normalize(lightBuffers[lightIndex].LightPosition.xyz - RHCenter); 	
	vec3 vAbs 					= abs(v.xyz);
	float faceIndex;
	if(vAbs.z >= vAbs.x && vAbs.z >= vAbs.y) 
		faceIndex 				= v.z < 0.0 ? 5.0 : 4.0;	
	else if(vAbs.y >= vAbs.x) 
		faceIndex 				= v.y < 0.0 ? 3.0 : 2.0;	
	else 
		faceIndex 				= v.x < 0.0 ? 1.0 : 0.0;
	const int face 				= int(faceIndex);
	const vec4 LightSpacePos 	= lightBuffers[lightIndex].LightVP[face] * vec4(RHCenter, 1);
	const vec3 ProjCoords 		= LightSpacePos.xyz / LightSpacePos.w;                                  
    const vec2 UVCoords 		= 0.5 * ProjCoords.xy + 0.5;
    RHUV 						= clamp(UVCoords, vec2(0.0f), vec2(1.0f));
	lightVP						= lightBuffers[lightIndex].LightVP[face];
	finalShadowSpot				= Shadow_Spot + face;
}

void CalcSpotShadow(in vec3 RHCenter, out vec2 RHUV, out mat4 lightVP, out int finalShadowSpot) 
{
	const vec4 LightSpacePos	= lightBuffers[lightIndex].LightVP[0] * vec4(RHCenter, 1);
	const vec3 ProjCoords 		= LightSpacePos.xyz / LightSpacePos.w;                                  
	const vec2 UVCoords 		= (0.5f * ProjCoords.xy + 0.5f);  
    RHUV 						= clamp(UVCoords, vec2(0.0f), vec2(1.0f));
	lightVP						= lightBuffers[lightIndex].LightVP[0];
	finalShadowSpot				= Shadow_Spot;
}

vec3 CalcShadowPos(in vec2 TexCoord, in int ShadowSpot, in mat4 InverseVP) 
{
	const vec3 uv_array_lookup	= vec3(TexCoord, ShadowSpot);
	const float depth			= texture(ShadowPos, uv_array_lookup).r; 
	const vec4 World_Pos		= InverseVP * vec4(vec3(TexCoord, depth) * 2.0f - 1.0f, 1.0f);
	return 						World_Pos.xyz / World_Pos.w;
}

void BounceFromShadow(in vec3 extents, in vec3 RHCellSize, in vec3 RHCenter, in vec2 RHUV, in mat4 InversePV, in int ShadowSpot, in sampler2DArray ShadowPos, in sampler2DArray ShadowNormal, in sampler2DArray ShadowFlux)
{
	// Variable Initialization
    float dist, dist_min = R_wcs, dist_max = 0.0f, dist_ave = 0.0, FF; 
	vec3 rsmColor, rsmPos, rsmNormal, color; 
    vec4 SH_dist_ave 			= vec4(0.0);
    vec4 SHr 					= vec4(0.0);  
    vec4 SHg 					= vec4(0.0); 
    vec4 SHb 					= vec4(0.0); 
	
	for (int i = 0; i < samples; ++i) 
    { 
		// produce a new sample location on the RSM texture
        vec3 rnd 				= 2.0f * texture(Noise, 14 * RHCenter / extents + vec3(i,0,0) / samples).xyz - 1.0f; 
		vec2 uv 				= RHUV + vec2( rnd.x * spread * cos(6.283 * rnd.y), rnd.x * spread * sin(6.283 * rnd.y) ); 		
		vec3 uv_array_lookup	= vec3(uv, ShadowSpot);
		
		rsmPos					= CalcShadowPos(uv, ShadowSpot, InversePV);
		rsmColor				= texture(ShadowFlux, uv_array_lookup).rgb * ColorModifier; 
		rsmNormal 				= normalize(texture(ShadowNormal, uv_array_lookup).rgb);		
		
        // produce a new sampling location in the RH stratum
		vec3 samplePos			= RHCenter + (0.5f * rnd) * RHCellSize;		
		
		// Normalize distance to RSM sample
        dist 					= distance(samplePos, rsmPos) / R_wcs; 
		
        // Determine the incident direction. 
        // Avoid very close samples (and numerical instability problems)
        vec3 dir 				= ( dist <= 0.007f ) ? vec3(0.0f) : normalize( samplePos - rsmPos );
		float dotprod 			= max(dot(dir, rsmNormal), 0.0f); 
		FF 						= dotprod / (0.1f + dist * dist);	
		
		#ifdef DEPTH_OCCL 
		// ---- Depth-buffer-based RSM sample occlusion
		// Initialize visibility to 1
		float depth_visibility = 1.0; 

		// set number of visibility samples along the line of sight. Can be set with #define
		float vis_samples = 8.0; // 3 to 8
		vec3 Qj; 
		vec3 Qcss; 
		
		// Check if the current RH point is hidden from view. If it is, then "visible" line-of-sight 
		// samples should also be hidden from view. If the RH point is visible to the camera, the 
		// same should hold for the visibility samples in order not to attenuate the light.
		Qcss = PointWCS2CSS(samplePos); 
		float rh_visibility = Qcss.z < (2.0 * texture(CameraDepth, vec3(0.5 * Qcss.xy + vec2(0.5), gl_Layer)).r -1.0) * 1.1 ? 1.0 : -1.0; 

		// Estimate attenuation along line of sight
		for (int j = 1; j < vis_samples; ++j) { 
			// determine next point along the line of sight
			Qj = rsmPos + (j / vis_samples) * (RHCenter - rsmPos); 
			Qcss = PointWCS2CSS(Qj); 
			// modulate the visibility according to the number of hidden LoS samples
			depth_visibility -= rh_visibility * Qcss.z < rh_visibility * (2.0 * texture(CameraDepth, 0.5 * Qcss.xy + vec2(0.5)).r - 1.0) ? 0.0 : 1.0 / vis_samples; 
		} 
		depth_visibility = clamp(depth_visibility, 0, 1);
		FF *= depth_visibility;
		#endif 
		
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
	SHr						   /= float( 3.14159f * float(samples) ); 
	SHg						   /= float( 3.14159f * float(samples) ); 
	SHb						   /= float( 3.14159f * float(samples) ); 
	dist_ave				   /= float( samples ); 	
	
	GI_Out1 					= vec4( dist_min, R_wcs - dist_max, dist_ave, 1.0f );
	GI_Out2 					= SHr; 
	GI_Out3 					= SHg; 
	GI_Out4 					= SHb; 
}

void main()
{		
	// Get current RH's world pos
	const vec3 bbox_max 		= BBox_Max.xyz;
	const vec3 bbox_min 		= BBox_Min.xyz;
	const vec3 pos				= vec3(gl_FragCoord.x, gl_FragCoord.y, gl_Layer);
	const vec3 extents 			= (bbox_max - bbox_min).xyz; 
	const vec3 RHCellSize		= extents / (resolution);
	const vec3 RHCenter 		= bbox_min + pos * RHCellSize; 	
	vec4 View_Pos				= vMatrix * vec4(RHCenter, 1);
	View_Pos.xyz				/= View_Pos.w;
	vec2 RHUV;
	mat4 lightVP;
	int finalShadowSpot;
		
	// RH -> light space, get sampling disk center	
	if (lightType == 0)
		CalcDirectionalShadow(RHCenter, -View_Pos.z, RHUV, lightVP, finalShadowSpot);
	else if (lightType == 1)
		CalcPointShadow(RHCenter, RHUV, lightVP, finalShadowSpot);
	else if (lightType == 2)
		CalcSpotShadow(RHCenter, RHUV, lightVP, finalShadowSpot);
		
	// Perform light bounce operation
	BounceFromShadow(extents, RHCellSize, RHCenter, RHUV, inverse(lightVP), finalShadowSpot, ShadowPos, ShadowNormal, ShadowFlux);	
}