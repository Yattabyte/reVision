/*
	Light_Directional

	- A directional light, mimicks the sun
*/

#pragma once
#ifndef LIGHT_DIRECTIONAL
#define LIGHT_DIRECTIONAL
#ifdef	DT_CORE_EXPORT
#define DELTA_CORE_API __declspec(dllexport)
#else
#define	DELTA_CORE_API __declspec(dllimport)
#endif
#define LIGHT_TYPE_DIRECTIONAL 0
#define NUM_CASCADES 4
#define GLM_FORCE_SWIZZLE
#define GLM_SWIZZLE_XYZ
#include "glm\glm.hpp"
#include "glm\gtc\quaternion.hpp"
#include "GLM\gtc\type_ptr.hpp"
#include "Entities\Light.h"

using namespace glm;

class Visibility_Token;
struct LightBuffer
{
	mat4 lightV;
	vec3 LightColor; float padding1;
	vec3 LightDirection; float padding2;
	float ShadowSize;
	float LightIntensity;
	int CascadeIndex;
	int Use_Shadows;

	// These need to be padded to 16 bytes each, because of layout std140 rules for array elements
	ivec4 Shadow_Spot[NUM_CASCADES]; // first element used only
	vec4 CascadeEndClipSpace[NUM_CASCADES]; // first element used only
	mat4 lightP[NUM_CASCADES]; // these are good already

	LightBuffer() {
		lightV = mat4(1.0f);
		LightColor = vec3(1.0f);
		LightDirection = vec3(0, -1, 0);
		ShadowSize = 0;
		LightIntensity = 0;
		CascadeIndex = 0;
		Use_Shadows = 0;
	}
};

class Light_Directional : public Light
{
public:
	/*************
	----Common----
	*************/

	DELTA_CORE_API ~Light_Directional();
	DELTA_CORE_API Light_Directional(const vec3 &rgb = vec3(1.0f), const float &ints = float(1.0f), const bool &use_shadows = true);
	DELTA_CORE_API Light_Directional(const Light_Directional &other);
	DELTA_CORE_API void operator= (const Light_Directional &other);
	DELTA_CORE_API virtual void registerSelf();
	DELTA_CORE_API virtual void unregisterSelf();


	/**********************
	----Light Functions----
	**********************/

	static int GetLightType() { return LIGHT_TYPE_DIRECTIONAL; }
	DELTA_CORE_API virtual void directPass(const int &vertex_count);
	DELTA_CORE_API virtual void indirectPass(const int &vertex_count);
	DELTA_CORE_API virtual void shadowPass(const Visibility_Token &vis_token) const;
	/*void shadowPass_models(VisibilityToken & vis_token);
	void shadowPass_terrain(VisibilityToken & vis_token);
	void shadowPass_brushes(VisibilityToken & vis_token);*/
	DELTA_CORE_API virtual bool shouldRender(const mat4 &PVMatrix);

	/*************************
	----Variable Functions----
	*************************/

	void setColor(const vec3 &rgb) { m_lightBuffer.LightColor = rgb; };
	vec3 getColor() const { return m_lightBuffer.LightColor; };
	void setIntensity(const float &ints) { m_lightBuffer.LightIntensity = ints; };
	float getIntensity() const { return m_lightBuffer.LightIntensity; };
	void setOrientation(const quat &ori) { m_orientation = ori; };
	quat getOrientation() const { return m_orientation; };
	bool CastsShadows() { return m_lightBuffer.Use_Shadows; };
	DELTA_CORE_API void Update();

private:
	/****************
	----Variables----
	****************/

	quat m_orientation;
	GLuint m_UBO;
	LightBuffer m_lightBuffer;
};

#endif // LIGHT_DIRECTIONAL