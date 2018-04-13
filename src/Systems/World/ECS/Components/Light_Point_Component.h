#pragma once
#ifndef LIGHT_POINT_COMPONENT
#define LIGHT_POINT_COMPONENT
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
#include "Utilities\GL\VectorBuffer.h"

using namespace glm;
class Shadow_FBO;
class System_World;
class Light_Point_Creator;
class EnginePackage;


/**
 * A renderable light component that mimics a light-bulb.
 * Uses dual paraboloid shadow maps.
 **/
class DT_ENGINE_API Light_Point_Component : protected Lighting_Component
{
public:
	// Interface Implementations
	virtual void receiveMessage(const ECSmessage &message);
	virtual void shadowPass();
	virtual bool isVisible(const mat4 & PMatrix, const mat4 &VMatrix);
	virtual float getImportance(const vec3 &position) const;
	

	/** Sends current data to the GPU. */
	void update();


protected:
	// (de)Constructors
	/** Destroys a point light component. */
	~Light_Point_Component();
	/** Constructs a point light component. */
	Light_Point_Component(const ECShandle &id, const ECShandle &pid, EnginePackage *enginePackage);


	// Protected Attributes
	// Shared Objects
	EnginePackage *m_enginePackage;
	Shadow_FBO *m_shadowMapper;
	System_World *m_world;
	// Cached attributes
	float m_radius;
	float m_squaredRadius;
	mat4 m_lightVMatrix; 
	vec3 m_lightPos;
	int m_shadowSpots[2];
	Camera m_camera[2];
	DynamicBuffer m_visGeoUBO[2], m_indirectGeo[2];
	friend class Light_Point_Creator;
};

class DT_ENGINE_API Light_Point_Creator : public ComponentCreator
{
public:
	Light_Point_Creator(ECSmessenger *ecsMessenger) : ComponentCreator(ecsMessenger) {}
	virtual Component* Create(const ECShandle &id, const ECShandle &pid, EnginePackage *enginePackage) {
		return new Light_Point_Component(id, pid, enginePackage);
	}
};

#endif // LIGHT_POINT_COMPONENT