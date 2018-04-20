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
	vec4 kernel[MAX_KERNEL_SIZE];
	float m_ssao_radius;
	int m_ssao_strength, m_aa_samples;
	int m_ssao;
};

/** Animated models uses this for transform + bone data. */
struct Geometry_Struct {
	#define NUM_MAX_BONES 100
	int useBones = 0;  // no padding here;
	GLuint materialID = 0; vec2 padding1; // for some reason padding here
	mat4 mMatrix = mat4(1.0f);
	mat4 bBoxMatrix = mat4(1.0f);
	mat4 transforms[NUM_MAX_BONES];
};

/** Directional lights use this for their lighting + transform data. */
struct Directional_Struct {
	#define NUM_CASCADES 4
	mat4 lightV = mat4(1.0f);
	vec3 LightColor = vec3(1.0f); float padding1;
	vec3 LightDirection = vec3(0, -1, 0); float padding2;
	float ShadowSize = 0;
	float LightIntensity = 0;
	int CascadeIndex = 0;
	int Use_Shadows = 1;

	// These need to be padded to 16 bytes each, because of layout std140 rules for array elements
	int Shadow_Spot[NUM_CASCADES]; // first element used only
	float CascadeEndClipSpace[NUM_CASCADES]; // first element used only
	mat4 lightP[NUM_CASCADES]; // these are good already
};

/** Point lights use this for their lighting + transform data. */
struct Point_Struct {
	mat4 mMatrix = mat4(1.0f);
	mat4 lightV = mat4(1.0f);
	vec3 LightColor = vec3(1.0f); float padding1;
	vec3 LightPosition = vec3(0.0f); float padding2;
	float p_far = 0;
	float ShadowSize = 0;
	float LightIntensity = 1;
	float LightRadius = 1;
	int Shadow_Spot1 = 0;
	int Shadow_Spot2 = 0;
	int Use_Shadows = 1;
	float padding;
};

/** Spot lights use this for their lighting + transform data. */
struct Spot_Struct {
	mat4 mMatrix = mat4(1.0f);
	mat4 lightV = mat4(1.0f);
	mat4 lightP = mat4(1.0f);
	vec3 LightColor = vec3(1.0f); float padding1;
	vec3 LightPosition = vec3(0.0f); float padding2;
	vec3 LightDirection = vec3(0, -1, 0); float padding3;
	float ShadowSize = 0;
	float LightIntensity = 0;
	float LightRadius = 0;
	float LightCutoff = 0;
	int Shadow_Spot = 0;
	int Use_Shadows = 0;
	vec2 padding;
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
	int samples = 16;
	int resolution = 16;
	float spread = 1.0f;
	float R_wcs = 1.0f;
	float factor = 1.0f;

	GI_Radiance_Struct(const int &res, const float &rad, const float &wrld, const float &blend, const int &smp) {
		resolution = res;
		spread = rad;
		R_wcs = wrld;
		factor = blend;
		samples = smp;
	}
};

#endif // GFX_DEFINES