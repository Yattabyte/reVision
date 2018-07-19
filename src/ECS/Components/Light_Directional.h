#pragma once
#ifndef LIGHT_DIRECTIONAL_COMPONENT_H
#define LIGHT_DIRECTIONAL_COMPONENT_H
#define GLM_FORCE_SWIZZLE
#define GLM_SWIZZLE_XYZ
#include "glm\glm.hpp"
#include "glm\gtc\quaternion.hpp"
#include "GLM\gtc\type_ptr.hpp"
#include "GL\glew.h"
#include "ECS\Components\Lighting.h"
#include "Systems\World\Camera.h"
#include "Systems\Graphics\Resources\GFX_DEFINES.h"
#include "Utilities\Transform.h"


class Directional_Tech;

/**
 * A renderable light component that mimics the sun.
 * Uses 4 cascaded shadowmaps.
 **/
class Light_Directional_C : public Lighting_C
{
public:
	// Interface implementations
	static const char * GetName() { return "Light_Directional"; }
	virtual float getImportance(const glm::vec3 & position) const;
	virtual bool isVisible(const float & radius, const glm::vec3 & eyePosition) const;
	virtual void occlusionPass(const unsigned int & type);
	virtual void shadowPass(const unsigned int & type);
	virtual	void update(const unsigned int & type);


	// Public Methods
	/** Recalculates the shadowmap cascades. */
	void calculateCascades();


protected:
	// (de)Constructors
	/** Destroys a directional light component. */
	~Light_Directional_C();
	/** Constructs a directional light component.
	 * @param	engine	the engine to use
	 * @param	color		the color to use
	 * @param	intensity	the intensity to use
	 * @param	transform	the transform to use */
	Light_Directional_C(Engine * engine, const glm::vec3 & color = glm::vec3(1.0f), const float & intensity = 1.0f, const Transform & transform = Transform());


	// Protected Attributes
	Directional_Tech * m_directionalTech;
	float m_cascadeEnd[5];
	int m_shadowSpot;
	size_t m_visSize[2];
	float m_shadowSize;
	glm::mat4 m_mMatrix;
	Camera m_camera;
	friend class ECS;


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

#endif // LIGHT_DIRECTIONAL_COMPONENT_H