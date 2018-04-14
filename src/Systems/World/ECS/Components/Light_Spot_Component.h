#pragma once
#ifndef LIGHT_SPOT_COMPONENT
#define LIGHT_SPOT_COMPONENT
#ifdef	ENGINE_EXE_EXPORT
#define DT_ENGINE_API 
#elif	ENGINE_DLL_EXPORT 
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
#include "Systems\World\ECS\Components\Lighting_Component.h"
#include "Systems\World\Camera.h"
#include "Utilities\GL\DynamicBuffer.h"

using namespace glm;
class Shadow_FBO;
class System_World;
class Light_Spot_Creator;
class EnginePackage;


/**
 * A renderable light component that mimics a flashlight.
 * Uses a single shadow map.
 **/
class DT_ENGINE_API Light_Spot_Component : protected Lighting_Component
{
public:
	// Interface Implementations
	virtual void receiveMessage(const ECSmessage &message);
	virtual void shadowPass();
	virtual bool isVisible(const mat4 & PMatrix, const mat4 &VMatrix);
	virtual float getImportance(const vec3 &position) const;
	virtual void update();


protected:
	// (de)Constructors
	/** Destroys a spot light component. */
	~Light_Spot_Component();
	/** Constructs a spot light component. */
	Light_Spot_Component(const ECShandle &id, const ECShandle &pid, EnginePackage *enginePackage);


	// Protected Attributes
	// Shared Objects
	EnginePackage *m_enginePackage;
	Shadow_FBO *m_shadowMapper;
	System_World *m_world;
	// Cached Attributes
	float m_radius;
	float m_squaredRadius;
	mat4 m_lightVMatrix;
	vec3 m_lightPos;
	int m_shadowSpot;
	quat m_orientation;
	Camera m_camera;
	DynamicBuffer m_visGeoUBO, m_indirectGeo;
	friend class Light_Spot_Creator;
};

class DT_ENGINE_API Light_Spot_Creator : public ComponentCreator
{
public:
	Light_Spot_Creator(ECSmessenger *ecsMessenger) : ComponentCreator(ecsMessenger) {}
	virtual Component* Create(const ECShandle &id, const ECShandle &pid, EnginePackage *enginePackage) {
		return new Light_Spot_Component(id, pid, enginePackage);
	}
};

#endif // LIGHT_SPOT_COMPONENT