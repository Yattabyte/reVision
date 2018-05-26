#pragma once
#ifndef GFX_DEFINES
#define GFX_DEFINES
#define GLM_FORCE_SWIZZLE
#define GLM_SWIZZLE_XYZ
#define GLEW_STATIC
#include "glm\glm.hpp"
#include "glm\gtc\quaternion.hpp"
#include "GLM\gtc\type_ptr.hpp"
#include "GL\glew.h"

#define ZERO_MEM(a) memset(a, 0, sizeof(a))
#define ARRAY_SIZE_IN_ELEMENTS(a) (sizeof(a)/sizeof(a[0]))

using namespace glm;

/*******************************************************************************************
					Structs for shipping data to GPU using GL buffers
*******************************************************************************************/

/** Graphics class uses this for viewport data. */
struct Renderer_Struct {
	#define MAX_KERNEL_SIZE 128
	float m_ssao_radius;
	int m_ssao_strength, m_aa_quality;
	int m_ssao;
	vec4 kernel[MAX_KERNEL_SIZE];
};

/** Animated models uses this for transform + bone data. */
struct Geometry_Dynamic_Struct {
	#define NUM_MAX_BONES 100
	int useBones = 0; 
	GLuint materialID = 0; vec2 padding1; 
	mat4 mMatrix = mat4(1.0f);
	mat4 bBoxMatrix = mat4(1.0f);
	mat4 transforms[NUM_MAX_BONES];
};

/** Static models uses this for transform data. */
struct Geometry_Static_Struct {
	int useBones = 0; 
	GLuint materialID = 0; vec2 padding1; 
	mat4 mMatrix = mat4(1.0f);
	mat4 bBoxMatrix = mat4(1.0f);
};

/** Directional lights use this for their lighting + transform data. */
struct Directional_Struct {
	#define NUM_CASCADES 4
	mat4 lightV = mat4(1.0f);
	vec3 LightColor = vec3(1.0f); float padding1;
	vec3 LightDirection = vec3(0, -1, 0); float padding2;
	float ShadowSize_Recip = 0;
	float LightIntensity = 0;
	int Shadow_Spot = 0;
	float CascadeEndClipSpace[NUM_CASCADES]; // first element used only
	float padding3; // end of scalars, pad by 2
	mat4 lightVP[NUM_CASCADES]; // these are good already
	mat4 inverseVP[NUM_CASCADES]; // these are good already
};

/** Cheap Directional lights use this for their lighting + transform data. */
struct Directional_Cheap_Struct {
	vec3 LightColor = vec3(1.0f); float padding1;
	vec3 LightDirection = vec3(0, -1, 0); float padding2;
	float LightIntensity = 0; vec3 padding3;
};

/** Point lights use this for their lighting + transform data. */
struct Point_Struct {
	mat4 mMatrix = mat4(1.0f);
	mat4 lightV = mat4(1.0f);
	mat4 lightPV[6];
	mat4 inversePV[6];
	vec3 LightColor = vec3(1.0f); float padding1;
	vec3 LightPosition = vec3(0.0f); float padding2;
	float ShadowSize_Recip = 0;
	float LightIntensity = 1;
	float LightRadius = 1;
	int Shadow_Spot;
};

/** Cheap Point lights use this for their lighting + transform data. */
struct Point_Cheap_Struct {
	mat4 mMatrix = mat4(1.0f);
	vec3 LightColor = vec3(1.0f); float padding1;
	vec3 LightPosition = vec3(0.0f); float padding2;
	float LightIntensity = 1;
	float LightRadius = 1;
	vec2 padding3;
};

/** Spot lights use this for their lighting + transform data. */
struct Spot_Struct {
	mat4 mMatrix = mat4(1.0f);
	mat4 lightV = mat4(1.0f);
	mat4 lightPV = mat4(1.0f);
	mat4 inversePV = mat4(1.0f);
	vec3 LightColor = vec3(1.0f); float padding1;
	vec3 LightPosition = vec3(0.0f); float padding2;
	vec3 LightDirection = vec3(0, -1, 0); float padding3;
	float ShadowSize_Recip = 0;
	float LightIntensity = 0;
	float LightRadius = 0;
	float LightCutoff = 0;
	int Shadow_Spot = 0;
	vec3 padding4;
};

/** Cheap Spot lights use this for their lighting + transform data. */
struct Spot_Cheap_Struct {
	mat4 mMatrix = mat4(1.0f);
	vec3 LightColor = vec3(1.0f); float padding1;
	vec3 LightPosition = vec3(0.0f); float padding2;
	vec3 LightDirection = vec3(0, -1, 0); float padding3;
	float LightIntensity = 0;
	float LightRadius = 0;
	float LightCutoff = 0;
	float padding4;
};

/** Reflectors use this for their transform data. */
struct Reflection_Struct {
	mat4 mMatrix = mat4(1.0f);
	vec4 BoxCamPos = vec4(0.0f);
	float Radius = 1.0f;
	int CubeSpot = 0;
	vec2 padding;
};

/** GI radiance hints class uses this for its viewport data. */
struct GI_Radiance_Struct {
	vec4 BBox_Max = vec4(1);
	vec4 BBox_Min = vec4(-1);
	int samples =  16;
	int resolution = 16;
	float spread = 1.0f;
	float R_wcs = 1.0f;
	float factor = 1.0f;

	GI_Radiance_Struct(const int &smp, const int &res, const float &rad, const float &wrld, const float &blend) {
		samples = smp;
		resolution = res;
		spread = rad;
		R_wcs = wrld;
		factor = blend;
	}
};

#endif // GFX_DEFINES