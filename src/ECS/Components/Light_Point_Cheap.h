#pragma once
#ifndef LIGHT_POINT_CHEAP_COMPONENT_H
#define LIGHT_POINT_CHEAP_COMPONENT_H
#define GLM_FORCE_SWIZZLE
#define GLM_SWIZZLE_XYZ
#include "glm\glm.hpp"
#include "glm\gtc\quaternion.hpp"
#include "GLM\gtc\type_ptr.hpp"
#include "GL\glew.h"
#include "ECS\Components\Lighting.h"
#include "Utilities\Transform.h"


/**
 * A renderable light component that mimics a light-bulb.
 * A cheap variation, no shadows or GI.
 **/
class Light_Point_Cheap_C : protected Lighting_C
{
public:
	// Interface Implementations
	virtual const char * getName() const { return "Light_Point_Cheap"; }
	virtual float getImportance(const glm::vec3 & position) const;
	virtual bool isVisible(const float & radius, const glm::vec3 & eyePosition) const;
	virtual void occlusionPass(const unsigned int & type) {}
	virtual void shadowPass(const unsigned int & type) {}
	virtual	void update(const unsigned int & type) {}


protected:
	// (de)Constructors
	/** Destroys a cheap point light component. */
	~Light_Point_Cheap_C();
	/** Constructs a cheap point light component. */
	Light_Point_Cheap_C(Engine * engine, const glm::vec3 & color = glm::vec3(1.0f), const float & intensity = 1.0f, const float & radius = 1.0f, const Transform & transform = Transform());
	

	// Protected Functions
	/** Recalculate matrices. */
	void updateViews();


	// Protected Attributes
	Engine * m_engine;
	// Cached attributes
	float m_radius, m_squaredRadius;
	glm::vec3 m_lightPos;
	friend class Component_Creator<Light_Point_Cheap_C>;


private:
	// Private Functions
	/** Set the light color to use.
	 * @param	color		the color to use */
	void setColor(const glm::vec3 & color);
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

#endif // LIGHT_POINT_CHEAP_COMPONENT_H