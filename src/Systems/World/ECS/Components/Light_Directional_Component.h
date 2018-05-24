#pragma once
#ifndef LIGHT_DIRECTIONAL_COMPONENT
#define LIGHT_DIRECTIONAL_COMPONENT
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
#include "Systems\Graphics\Resources\GFX_DEFINES.h"

using namespace glm;
class Directional_Tech;
class Light_Directional_Creator;
class EnginePackage;


/**
 * A renderable light component that mimics the sun.
 * Uses 4 cascaded shadowmaps.
 **/
class DT_ENGINE_API Light_Directional_Component : protected Lighting_Component
{
public:
	// Interface implementations
	virtual void receiveMessage(const ECSmessage &message);
	virtual bool isVisible(const float & radius, const vec3 & eyePosition) const;
	virtual void occlusionPass();
	virtual void shadowPass();
	virtual float getImportance(const vec3 &position) const;
	virtual	void update();


	// Public Methods
	/** Recalculates the shadowmap cascades. */
	void calculateCascades();


protected:
	// (de)Constructors
	/** Destroys a directional light component. */
	~Light_Directional_Component();
	/** Constructs a directional light component. */
	Light_Directional_Component(const ECShandle &id, const ECShandle &pid, EnginePackage *enginePackage);


	// Protected Attributes
	EnginePackage *m_enginePackage;
	Directional_Tech * m_directionalTech;
	float m_cascadeEnd[5];
	int m_shadowSpot;
	size_t m_visSize;
	float m_shadowSize;
	mat4 m_mMatrix;
	Camera m_camera;
	friend class Light_Directional_Creator;
};

class DT_ENGINE_API Light_Directional_Creator : public ComponentCreator
{
public:
	Light_Directional_Creator(ECSmessenger *ecsMessenger) : ComponentCreator(ecsMessenger) {}
	virtual Component* Create(const ECShandle &id, const ECShandle &pid, EnginePackage *enginePackage) {
		return new Light_Directional_Component(id, pid, enginePackage);
	}
};

#endif // LIGHT_DIRECTIONAL_COMPONENT