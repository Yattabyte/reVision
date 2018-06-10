#pragma once
#ifndef LIGHT_DIRECTIONAL_COMPONENT_H
#define LIGHT_DIRECTIONAL_COMPONENT_H
#define GLM_FORCE_SWIZZLE
#define GLM_SWIZZLE_XYZ
#include "glm\glm.hpp"
#include "glm\gtc\quaternion.hpp"
#include "GLM\gtc\type_ptr.hpp"
#include "GL\glew.h"
#include "Systems\World\ECS\Components\Lighting_Component.h"
#include "Systems\World\Camera.h"
#include "Systems\Graphics\Resources\GFX_DEFINES.h"
#include "Utilities\Transform.h"

using namespace glm;
class Directional_Tech;
class Light_Directional_Creator;
class EnginePackage;


/**
 * A renderable light component that mimics the sun.
 * Uses 4 cascaded shadowmaps.
 **/
class Light_Directional_Component : protected Lighting_Component
{
public:
	// Interface implementations
	virtual const char * getName() const { return "Light_Directional"; }
	virtual float getImportance(const vec3 & position) const;
	virtual bool isVisible(const float & radius, const vec3 & eyePosition) const;
	virtual void occlusionPass(const unsigned int & type);
	virtual void shadowPass(const unsigned int & type);
	virtual	void update(const unsigned int & type);


	// Public Methods
	/** Recalculates the shadowmap cascades. */
	void calculateCascades();


protected:
	// (de)Constructors
	/** Destroys a directional light component. */
	~Light_Directional_Component();
	/** Constructs a directional light component. */
	Light_Directional_Component(EnginePackage *enginePackage);


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


private:
	// Private Functions
	/** Set the light color to use. 
	 * @param	color		the color to use */
	void setColor(const vec3 & color);
	/** Set the light intensity to use.
	 * @param	intensity	the intensity to use */
	void setIntensity(const float & intensity);
	/** Set the transformation for this component.
	 * @param	transform	the transform to use */
	void setTransform(const Transform & transform);
};

class Light_Directional_Creator : public ComponentCreator
{
public:
	Light_Directional_Creator() : ComponentCreator() {}
	virtual Component* create(EnginePackage *enginePackage) {
		return new Light_Directional_Component(enginePackage);
	}
};

#endif // LIGHT_DIRECTIONAL_COMPONENT_H