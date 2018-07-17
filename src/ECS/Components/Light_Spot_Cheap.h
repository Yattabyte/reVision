#pragma once
#ifndef LIGHT_SPOT_CHEAP_COMPONENT_H
#define LIGHT_SPOT_CHEAP_COMPONENT_H
#define GLM_FORCE_SWIZZLE
#define GLM_SWIZZLE_XYZ
#include "glm\glm.hpp"
#include "glm\gtc\quaternion.hpp"
#include "GLM\gtc\type_ptr.hpp"
#include "GL\glew.h"
#include "ECS\Components\Lighting.h"
#include "Utilities\Transform.h"


class Light_Spot_Cheap_Creator;
class Engine;

/**
 * A renderable light component that mimics a flashlight.
 * A cheap variation, no shadows or GI.
 **/
class Light_Spot_Cheap_C : protected Lighting_C
{
public:
	// Interface Implementations
	virtual const char * getName() const { return "Light_Spot_Cheap"; }
	virtual float getImportance(const glm::vec3 & position) const;
	virtual bool isVisible(const float & radius, const glm::vec3 & eyePosition) const;
	virtual void occlusionPass(const unsigned int & type) {}
	virtual void shadowPass(const unsigned int & type) {}
	virtual	void update(const unsigned int & type) {}


protected:
	// (de)Constructors
	/** Destroys a spot light component. */
	~Light_Spot_Cheap_C();
	/** Constructs a spot light component. */
	Light_Spot_Cheap_C(Engine *engine);


	// Protected Functions
	/** Recalculate matrices. */
	void updateViews();


	// Protected Attributes
	// Cached Attributes
	Engine * m_engine;
	float m_radius;
	float m_squaredRadius;
	glm::quat m_orientation;
	glm::vec3 m_lightPos;
	friend class Light_Spot_Cheap_Creator;


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
	/** Set the light cutoff to use.
	 * @param	cutoff		the cutoff to use */
	void setCutoff(const float & cutoff);
	/** Set the transformation for this component.
	 * @param	transform	the transform to use */
	void setTransform(const Transform & transform);
};

class Light_Spot_Cheap_Creator : public ComponentCreator
{
public:
	Light_Spot_Cheap_Creator() : ComponentCreator() {}
	virtual Component* create(Engine *engine) {
		return new Light_Spot_Cheap_C(engine);
	}
};

#endif // LIGHT_SPOT_CHEAP_COMPONENT_H