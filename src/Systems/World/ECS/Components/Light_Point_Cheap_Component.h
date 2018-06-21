#pragma once
#ifndef LIGHT_POINT_CHEAP_COMPONENT_H
#define LIGHT_POINT_CHEAP_COMPONENT_H
#define GLM_FORCE_SWIZZLE
#define GLM_SWIZZLE_XYZ
#include "glm\glm.hpp"
#include "glm\gtc\quaternion.hpp"
#include "GLM\gtc\type_ptr.hpp"
#include "GL\glew.h"
#include "Systems\World\ECS\Components\Lighting_Component.h"
#include "Utilities\Transform.h"


using namespace glm;
class Light_Point_Cheap_Creator;
class EnginePackage;

/**
 * A renderable light component that mimics a light-bulb.
 * A cheap variation, no shadows or GI.
 **/
class Light_Point_Cheap_Component : protected Lighting_Component
{
public:
	// Interface Implementations
	virtual const char * getName() const { return "Light_Point_Cheap"; }
	virtual float getImportance(const vec3 & position) const;
	virtual bool isVisible(const float & radius, const vec3 & eyePosition) const;
	virtual void occlusionPass(const unsigned int & type) {}
	virtual void shadowPass(const unsigned int & type) {}
	virtual	void update(const unsigned int & type) {}


protected:
	// (de)Constructors
	/** Destroys a cheap point light component. */
	~Light_Point_Cheap_Component();
	/** Constructs a cheap point light component. */
	Light_Point_Cheap_Component(EnginePackage *enginePackage);
	

	// Protected Functions
	/** Recalculate matrices. */
	void updateViews();


	// Protected Attributes
	EnginePackage * m_enginePackage;
	// Cached attributes
	float m_radius, m_squaredRadius;
	vec3 m_lightPos;
	friend class Light_Point_Cheap_Creator;


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
	/** Set the transformation for this component.
	 * @param	transform	the transform to use */
	void setTransform(const Transform & transform);
};

class Light_Point_Cheap_Creator : public ComponentCreator
{
public:
	Light_Point_Cheap_Creator() : ComponentCreator() {}
	virtual Component* create(EnginePackage * enginePackage) {
		return new Light_Point_Cheap_Component(enginePackage);
	}
};

#endif // LIGHT_POINT_CHEAP_COMPONENT_H