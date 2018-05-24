#pragma once
#ifndef LIGHT_SPOT_CHEAP_COMPONENT
#define LIGHT_SPOT_CHEAP_COMPONENT
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


using namespace glm;
class Light_Spot_Cheap_Creator;
class EnginePackage;

/**
 * A renderable light component that mimics a flashlight.
 * A cheap variation, no shadows or GI.
 **/
class DT_ENGINE_API Light_Spot_Cheap_Component : protected Lighting_Component
{
public:
	// Interface Implementations
	virtual void receiveMessage(const ECSmessage &message);
	virtual bool isVisible(const float & radius, const vec3 & eyePosition) const;
	virtual float getImportance(const vec3 &position) const;
	virtual void occlusionPass() {}
	virtual void shadowPass() {}
	virtual	void update() {}


protected:
	// (de)Constructors
	/** Destroys a spot light component. */
	~Light_Spot_Cheap_Component();
	/** Constructs a spot light component. */
	Light_Spot_Cheap_Component(const ECShandle &id, const ECShandle &pid, EnginePackage *enginePackage);


	// Protected Functions
	/** Recalculate matrices. */
	void updateViews();


	// Protected Attributes
	// Cached Attributes
	float m_radius;
	float m_squaredRadius;
	quat m_orientation;
	vec3 m_lightPos;
	friend class Light_Spot_Cheap_Creator;
};

class DT_ENGINE_API Light_Spot_Cheap_Creator : public ComponentCreator
{
public:
	Light_Spot_Cheap_Creator(ECSmessenger *ecsMessenger) : ComponentCreator(ecsMessenger) {}
	virtual Component* Create(const ECShandle &id, const ECShandle &pid, EnginePackage *enginePackage) {
		return new Light_Spot_Cheap_Component(id, pid, enginePackage);
	}
};

#endif // LIGHT_SPOT_CHEAP_COMPONENT