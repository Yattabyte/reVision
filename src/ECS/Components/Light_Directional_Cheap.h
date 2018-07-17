#pragma once
#ifndef LIGHT_DIRECTIONAL_CHEAP_COMPONENT_H
#define LIGHT_DIRECTIONAL_CHEAP_COMPONENT_H
#define GLM_FORCE_SWIZZLE
#define GLM_SWIZZLE_XYZ
#include "glm\glm.hpp"
#include "glm\gtc\quaternion.hpp"
#include "GLM\gtc\type_ptr.hpp"
#include "GL\glew.h"
#include "ECS\Components\Lighting.h"
#include "Utilities\Transform.h"


class Light_Directional_Cheap_Creator;
class Engine;

/**
 * A renderable light component that mimics the sun.
 * A cheap variation, no shadows or GI.
 **/
class Light_Directional_Cheap_C : protected Lighting_C
{
public:
	// Interface implementations
	virtual const char * getName() const { return "Light_Directional_Cheap"; }
	virtual float getImportance(const glm::vec3 & position) const;
	virtual bool isVisible(const float & radius, const glm::vec3 & eyePosition) const;
	virtual void occlusionPass(const unsigned int & type) {}
	virtual void shadowPass(const unsigned int & type) {}
	virtual	void update(const unsigned int & type) {}


protected:
	// (de)Constructors
	/** Destroys a cheap directional light component. */
	~Light_Directional_Cheap_C();
	/** Constructs a cheap directional light component. */
	Light_Directional_Cheap_C(Engine * engine, const glm::vec3 & color = glm::vec3(1.0f), const float & intensity = 1.0f, const Transform & transform = Transform());


	// Protected Attributes
	Engine * m_engine;
	friend class Light_Directional_Cheap_Creator;


private:
	// Private Functions
	/** Set the light color to use.
	* @param	color		the color to use */
	void setColor(const glm::vec3 & color);
	/** Set the light intensity to use.
	* @param	intensity	the intensity to use */
	void setIntensity(const float & intensity);
	/** Set the transformation for this component.
	* @param	transform	the transform to use */
	void setTransform(const Transform & transform);
};

class Light_Directional_Cheap_Creator : public ComponentCreator
{
public:
	Light_Directional_Cheap_Creator() : ComponentCreator() {}
	virtual Component* create(Engine *engine) {
		return new Light_Directional_Cheap_C(engine);
	}
};

#endif // LIGHT_DIRECTIONAL_CHEAP_COMPONENT_H