/*
	Light_Point_Component

	- A lighting technique that mimicks a lightbulb / pointlight
*/

#pragma once
#ifndef LIGHT_POINT_COMPONENT
#define LIGHT_POINT_COMPONENT
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

struct LightPointBuffer
{
	mat4 mMatrix;
	mat4 lightV;
	vec3 LightColor; float padding1;
	vec3 LightPosition; float padding2;
	float p_far;
	float p_dir;
	float ShadowSize;
	float LightIntensity;
	float LightRadius;
	int Shadow_Spot1;
	int Shadow_Spot2;
	int Use_Shadows;
	int LightStencil;
	
	LightPointBuffer() {
		mMatrix = mat4(1.0f);
		lightV = mat4(1.0f);
		LightColor = vec3(1.0f);
		LightPosition = vec3(0.0f);
		p_far = 0;
		p_dir = 0;
		ShadowSize = 0;
		LightIntensity = 0;
		LightRadius = 0;
		Shadow_Spot1 = 0;
		Shadow_Spot2 = 0;
		Use_Shadows = 0;
		LightStencil = 0;
	}
};

class System_Shadowmap;
class Light_Point_Creator;
class Engine_Package;
class DT_ENGINE_API Light_Point_Component : protected Lighting_Component
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
		SET_POSITION,
		SET_TRANSFORM
	};


	/***************************
	----Light_Point Functions----
	***************************/

	// Direct lighting pass
	virtual void directPass(const int &vertex_count);
	// Indirect lighting pass
	virtual void indirectPass(const int &vertex_count);
	// Shadow lighting pass
	virtual void shadowPass();
	// Returns whether or not this light is visible
	virtual bool IsVisible(const mat4 & PVMatrix);
	// Sends current data to the GPU
	void Update();


protected:
	~Light_Point_Component();
	Light_Point_Component(const ECShandle &id, const ECShandle &pid, Engine_Package *enginePackage);
	GLuint m_uboID;
	LightPointBuffer m_uboData;
	Engine_Package *m_enginePackage;
	float m_squaredRadius;
	Camera m_camera[2];
	friend class Light_Point_Creator;
};

class DT_ENGINE_API Light_Point_Creator : public ComponentCreator
{
public:
	Light_Point_Creator(ECSmessanger *ecsMessanger) : ComponentCreator(ecsMessanger) {}
	virtual Component* Create(const ECShandle &id, const ECShandle &pid, Engine_Package *enginePackage) {
		return new Light_Point_Component(id, pid, enginePackage);
	}
};

#endif // LIGHT_POINT_COMPONENT