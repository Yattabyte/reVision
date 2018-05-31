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
#include "Utilities\Transform.h"

using namespace glm;
class Spot_Tech;
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
	virtual const char * getName() const { return "Light_Spot"; }
	virtual bool isVisible(const float & radius, const vec3 & eyePosition) const;
	virtual void occlusionPass();
	virtual void shadowPass();
	virtual float getImportance(const vec3 &position) const;
	virtual void update();


protected:
	// (de)Constructors
	/** Destroys a spot light component. */
	~Light_Spot_Component();
	/** Constructs a spot light component. */
	Light_Spot_Component(EnginePackage *enginePackage);


	// Protected Functions
	/** Recalculate matrices. */
	void updateViews();


	// Protected Attributes
	// Shared Objects
	EnginePackage *m_enginePackage;
	Spot_Tech * m_spotTech;
	System_World *m_world;
	// Cached Attributes
	float m_radius;
	float m_squaredRadius;
	mat4 m_lightVMatrix, m_lightPV;
	vec3 m_lightPos;
	int m_shadowSpot;
	quat m_orientation;
	Camera m_camera;
	size_t m_visSize;
	friend class Light_Spot_Creator;


private:
	// Private Functions
	/** Set the light color to use.
	 * @param	color		the color to use */
	void setColor(const vec3 & color);
	/** Set the light intensity to use.
	 * @param	intensity	the intensity to use */
	void setIntensity(const float & intensity);
	/** Set the light radius to use.
	 * @param	radius		the radius to use */
	void setRadius(const float & radius);
	/** Set the light cutoff to use.
	 * @param	cutoff		the cutoff to use */
	void setCutoff(const float & cutoff);
	/** Set the transformation for this component.
	 * @param	transform	the transform to use */
	void setTransform(const Transform & transform);
};

class DT_ENGINE_API Light_Spot_Creator : public ComponentCreator
{
public:
	Light_Spot_Creator() : ComponentCreator() {}
	virtual Component* create(EnginePackage *enginePackage) {
		return new Light_Spot_Component(enginePackage);
	}
};

#endif // LIGHT_SPOT_COMPONENT