#pragma once
#ifndef LIGHT_SPOT_COMPONENT_H
#define LIGHT_SPOT_COMPONENT_H
#define GLM_FORCE_SWIZZLE
#define GLM_SWIZZLE_XYZ
#include "glm\glm.hpp"
#include "glm\gtc\quaternion.hpp"
#include "GLM\gtc\type_ptr.hpp"
#include "GL\glew.h"
#include "ECS\Components\Lighting.h"
#include "Systems\World\Camera.h"
#include "Utilities\Transform.h"


class Spot_Tech;
class System_World;
class Engine;

/**
 * A renderable light component that mimics a flashlight.
 * Uses a single shadow map.
 **/
class Light_Spot_C : public Lighting_C
{
public:
	// Interface Implementations
	static const char * GetName() { return "Light_Spot"; }
	static std::vector<const char *> GetParamTypes() {
		static std::vector<const char *> params = { "vec3", "float", "float", "float", "transform" };
		return params;
	}
	virtual float getImportance(const glm::vec3 & position) const;
	virtual bool isVisible(const float & radius, const glm::vec3 & eyePosition) const;
	virtual void occlusionPass(const unsigned int & type);
	virtual void shadowPass(const unsigned int & type);
	virtual void update(const unsigned int & type);


protected:
	// (de)Constructors
	/** Destroys a spot light component. */
	~Light_Spot_C();
	/** Construct by means of an argument list. */
	Light_Spot_C(Engine * engine, const ArgumentList & argumentList);;
	/** Constructs a spot light component.
	 * @param	engine	the engine to use
	 * @param	color		the color to use
	 * @param	intensity	the intensity to use
	 * @param	radius		the radius to use
	 * @param	cutoff		the cutoff to use 
	 * @param	transform	the transform to use */
	Light_Spot_C(Engine * engine, const glm::vec3 & color = glm::vec3(1.0f), const float & intensity = 1.0f, const float & radius = 1.0f, const float & cutoff = 1.0f, const Transform & transform = Transform());


	// Protected Functions
	/** Recalculate matrices. */
	void updateViews();


	// Protected Attributes
	// Shared Objects
	Spot_Tech * m_spotTech;
	System_World *m_world;
	// Cached Attributes
	float m_radius;
	float m_squaredRadius;
	glm::mat4 m_lightVMatrix, m_lightPV;
	glm::vec3 m_lightPos;
	int m_shadowSpot;
	glm::quat m_orientation;
	Camera m_camera;
	size_t m_visSize[2];
	friend class ECS;
	friend class Component_Creator<Light_Spot_C>;


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

#endif // LIGHT_SPOT_COMPONENT_H