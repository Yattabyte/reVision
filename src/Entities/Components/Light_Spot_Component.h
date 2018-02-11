/*
	Light_Spot_Component

	- A lighting technique that mimicks a flashlight / spotlight
*/

#pragma once
#ifndef LIGHT_SPOT_COMPONENT
#define LIGHT_SPOT_COMPONENT
#ifdef	ENGINE_EXPORT
#define DT_ENGINE_API __declspec(dllexport)
#else            
#define	DT_ENGINE_API __declspec(dllimport)
#endif
#define GLM_FORCE_SWIZZLE
#define GLM_SWIZZLE_XYZ
#include "glm\glm.hpp"
#include "glm\gtc\quaternion.hpp"
#include "GLM\gtc\type_ptr.hpp"
#include "GL\glew.h"
#include "Entities\Components\Lighting_Component.h"
#include "Systems\World\Camera.h"

using namespace glm;

struct LightSpotBuffer
{
	mat4 mMatrix;
	mat4 lightV;
	mat4 lightP;
	vec3 LightColor; float padding1;
	vec3 LightPosition; float padding2;
	vec3 LightDirection; float padding3;
	float ShadowSize;
	float LightIntensity;
	float LightRadius;
	float LightCutoff;
	int Shadow_Spot;
	int Use_Shadows;
	int LightStencil;
	
	LightSpotBuffer() {
		mMatrix = mat4(1.0f);
		lightV = mat4(1.0f);
		lightP = mat4(1.0f);
		LightColor = vec3(1.0f);
		LightPosition = vec3(0.0f);
		LightDirection = vec3(0, -1, 0);
		ShadowSize = 0;
		LightIntensity = 0;
		LightRadius = 0;
		LightCutoff = 0;
		Shadow_Spot = 0;
		Use_Shadows = 0;
		LightStencil = 0;
	}
};

class System_Shadowmap;
class System_World;
class Light_Spot_Creator;
class EnginePackage;
class DT_ENGINE_API Light_Spot_Component : protected Lighting_Component
{
public:
	/*************
	----Common----
	*************/

	// Logic for interpreting receiving messages
	virtual void ReceiveMessage(const ECSmessage &message);
	static enum MSG_TYPES
	{
		SET_COLOR,
		SET_INTENSITY,
		SET_RADIUS,
		SET_CUTOFF,
		SET_POSITION,
		SET_ORIENTATION,
		SET_TRANSFORM,
	};
	

	/***************************
	----Light_Spot Functions----
	***************************/

	// Direct lighting pass
	void directPass(const int &vertex_count);
	// Indirect lighting pass
	void indirectPass(const int &vertex_count);
	// Shadow lighting pass
	void shadowPass();
	// Returns whether or not this light is visible
	bool IsVisible(const mat4 & PMatrix, const mat4 &VMatrix);
	// Returns the importance value for this light (distance / size)
	float getImportance(const vec3 &position);
	// Returns the shadow spot for this light
	GLuint getShadowSpot() const;
	// Sends current data to the GPU
	void Update();


protected:
	~Light_Spot_Component();
	Light_Spot_Component(const ECShandle &id, const ECShandle &pid, EnginePackage *enginePackage);
	GLuint m_uboID;
	LightSpotBuffer m_uboData;
	EnginePackage *m_enginePackage;
	System_Shadowmap *m_shadowMapper;
	System_World *m_world;
	quat m_orientation;
	float m_squaredRadius;
	Camera m_camera;
	friend class Light_Spot_Creator;
};

class DT_ENGINE_API Light_Spot_Creator : public ComponentCreator
{
public:
	Light_Spot_Creator(ECSmessanger *ecsMessanger) : ComponentCreator(ecsMessanger) {}
	virtual Component* Create(const ECShandle &id, const ECShandle &pid, EnginePackage *enginePackage) {
		return new Light_Spot_Component(id, pid, enginePackage);
	}
};

#endif // LIGHT_SPOT_COMPONENT