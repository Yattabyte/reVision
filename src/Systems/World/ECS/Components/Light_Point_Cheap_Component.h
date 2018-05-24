#pragma once
#ifndef LIGHT_POINT_CHEAP_COMPONENT
#define LIGHT_POINT_CHEAP_COMPONENT
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
class Light_Point_Cheap_Creator;
class EnginePackage;

/**
 * A renderable light component that mimics a light-bulb.
 * A cheap variation, no shadows or GI.
 **/
class DT_ENGINE_API Light_Point_Cheap_Component : protected Lighting_Component
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
	/** Destroys a cheap point light component. */
	~Light_Point_Cheap_Component();
	/** Constructs a cheap point light component. */
	Light_Point_Cheap_Component(const ECShandle &id, const ECShandle &pid, EnginePackage *enginePackage);
	

	// Protected Functions
	/** Recalculate matrices. */
	void updateViews();


	// Protected Attributes
	// Cached attributes
	float m_radius, m_squaredRadius;
	vec3 m_lightPos;
	friend class Light_Point_Cheap_Creator;
};

class DT_ENGINE_API Light_Point_Cheap_Creator : public ComponentCreator
{
public:
	Light_Point_Cheap_Creator(ECSmessenger * ecsMessenger) : ComponentCreator(ecsMessenger) {}
	virtual Component* Create(const ECShandle & id, const ECShandle & pid, EnginePackage * enginePackage) {
		return new Light_Point_Cheap_Component(id, pid, enginePackage);
	}
};

#endif // LIGHT_POINT_CHEAP_COMPONENT