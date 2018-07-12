#pragma once
#ifndef GFX_DEFINES_H
#define GFX_DEFINES_H
#define GLM_FORCE_SWIZZLE
#define GLM_SWIZZLE_XYZ
#define GLEW_STATIC
#include "glm\glm.hpp"
#include "glm\gtc\quaternion.hpp"
#include "GLM\gtc\type_ptr.hpp"
#include "GL\glew.h"

#define ZERO_MEM(a) memset(a, 0, sizeof(a))
#define ARRAY_SIZE_IN_ELEMENTS(a) (sizeof(a)/sizeof(a[0]))


/*******************************************************************************************
					Structs for shipping data to GPU using GL buffers
*******************************************************************************************/

/** Graphics class uses this for viewport data. */
struct Renderer_Struct {
	#define MAX_KERNEL_SIZE 128
	float m_ssao_radius;
	int m_ssao_strength, m_aa_quality;
	int m_ssao;
	glm::vec4 kernel[MAX_KERNEL_SIZE];
};

/** Animated models uses this for transform + bone data. */
struct Geometry_Dynamic_Struct {
	#define NUM_MAX_BONES 100
	int useBones = 0; 
	GLuint materialID = 0; glm::vec2 padding1; 
	glm::mat4 mMatrix = glm::mat4(1.0f);
	glm::mat4 bBoxMatrix = glm::mat4(1.0f);
	glm::mat4 transforms[NUM_MAX_BONES];
};

/** Static models uses this for transform data. */
struct Geometry_Static_Struct {
	GLuint materialID = 0; glm::vec3 padding1; 
	glm::mat4 mMatrix = glm::mat4(1.0f);
	glm::mat4 bBoxMatrix = glm::mat4(1.0f);
};

/** Directional lights use this for their lighting + transform data. */
struct Directional_Struct {
	#define NUM_CASCADES 4
	glm::mat4 lightV = glm::mat4(1.0f);
	glm::vec3 LightColor = glm::vec3(1.0f); float padding1;
	glm::vec3 LightDirection = glm::vec3(0, -1, 0); float padding2;
	float ShadowSize_Recip = 0;
	float LightIntensity = 0;
	int Shadow_Spot = 0;
	float CascadeEndClipSpace[NUM_CASCADES]; // first element used only
	float padding3; // end of scalars, pad by 2
	glm::mat4 lightVP[NUM_CASCADES]; // these are good already
	glm::mat4 inverseVP[NUM_CASCADES]; // these are good already
};

/** Cheap Directional lights use this for their lighting + transform data. */
struct Directional_Cheap_Struct {
	glm::vec3 LightColor = glm::vec3(1.0f); float padding1;
	glm::vec3 LightDirection = glm::vec3(0, -1, 0); float padding2;
	float LightIntensity = 0; glm::vec3 padding3;
};

/** Point lights use this for their lighting + transform data. */
struct Point_Struct {
	glm::mat4 mMatrix = glm::mat4(1.0f);
	glm::mat4 lightV = glm::mat4(1.0f);
	glm::mat4 lightPV[6];
	glm::mat4 inversePV[6];
	glm::vec3 LightColor = glm::vec3(1.0f); float padding1;
	glm::vec3 LightPosition = glm::vec3(0.0f); float padding2;
	float ShadowSize_Recip = 0;
	float LightIntensity = 1;
	float LightRadius = 1;
	int Shadow_Spot;
};

/** Cheap Point lights use this for their lighting + transform data. */
struct Point_Cheap_Struct {
	glm::mat4 mMatrix = glm::mat4(1.0f);
	glm::vec3 LightColor = glm::vec3(1.0f); float padding1;
	glm::vec3 LightPosition = glm::vec3(0.0f); float padding2;
	float LightIntensity = 1;
	float LightRadius = 1;
	glm::vec2 padding3;
};

/** Spot lights use this for their lighting + transform data. */
struct Spot_Struct {
	glm::mat4 mMatrix = glm::mat4(1.0f);
	glm::mat4 lightV = glm::mat4(1.0f);
	glm::mat4 lightPV = glm::mat4(1.0f);
	glm::mat4 inversePV = glm::mat4(1.0f);
	glm::vec3 LightColor = glm::vec3(1.0f); float padding1;
	glm::vec3 LightPosition = glm::vec3(0.0f); float padding2;
	glm::vec3 LightDirection = glm::vec3(0, -1, 0); float padding3;
	float ShadowSize_Recip = 0;
	float LightIntensity = 0;
	float LightRadius = 0;
	float LightCutoff = 0;
	int Shadow_Spot = 0;
	glm::vec3 padding4;
};

/** Cheap Spot lights use this for their lighting + transform data. */
struct Spot_Cheap_Struct {
	glm::mat4 mMatrix = glm::mat4(1.0f);
	glm::vec3 LightColor = glm::vec3(1.0f); float padding1;
	glm::vec3 LightPosition = glm::vec3(0.0f); float padding2;
	glm::vec3 LightDirection = glm::vec3(0, -1, 0); float padding3;
	float LightIntensity = 0;
	float LightRadius = 0;
	float LightCutoff = 0;
	float padding4;
};

/** Reflectors use this for their transform data. */
struct Reflection_Struct {
	glm::mat4 mMatrix = glm::mat4(1.0f);
	glm::mat4 rotMatrix = glm::mat4(1.0f);
	glm::vec4 BoxCamPos = glm::vec4(0.0f);
	glm::vec4 BoxScale = glm::vec4(0.0f);
	int CubeSpot = 0;
	glm::vec3 padding;
};

/** GI radiance hints class uses this for its viewport data. */
struct GI_Radiance_Struct {
	glm::vec4 BBox_Max = glm::vec4(1);
	glm::vec4 BBox_Min = glm::vec4(-1);
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

#endif // GFX_DEFINES_H