#pragma once
#ifndef LIGHT_POINT_COMPONENT_H
#define LIGHT_POINT_COMPONENT_H
#define GLM_FORCE_SWIZZLE
#define GLM_SWIZZLE_XYZ
#include "glm\glm.hpp"
#include "glm\gtc\quaternion.hpp"
#include "GLM\gtc\type_ptr.hpp"
#include "GL\glew.h"
#include "ECS\Components\Lighting.h"
#include "Systems\World\Camera.h"
#include "Utilities\Transform.h"


class Point_Tech;
class System_World;
class Light_Point_Creator;
class Engine;

/**
 * A renderable light component that mimics a light-bulb.
 * Uses 6 shadow maps.
 **/
class Light_Point_C : protected Lighting_C
{
public:
	// Interface Implementations
	virtual const char * getName() const { return "Light_Point"; }
	virtual float getImportance(const glm::vec3 & position) const;
	virtual bool isVisible(const float & radius, const glm::vec3 & eyePosition) const;
	virtual void occlusionPass(const unsigned int & type);
	virtual void shadowPass(const unsigned int & type);
	virtual void update(const unsigned int & type);


protected:
	// (de)Constructors
	/** Destroys a point light component. */
	~Light_Point_C();
	/** Constructs a point light component. */
	Light_Point_C(Engine * engine, const glm::vec3 & color = glm::vec3(1.0f), const float & intensity = 1.0f, const float & radius = 1.0f, const Transform & transform = Transform());


	// Protected Functions
	/** Recalculate matrices. */
	void updateViews();


	// Protected Attributes
	// Shared Objects
	Engine * m_engine;
	Point_Tech * m_pointTech;
	System_World * m_world;
	// Cached attributes
	float m_radius;
	float m_squaredRadius;
	glm::mat4 m_lightVMatrix; 
	glm::vec3 m_lightPos;
	int m_shadowSpot;
	Camera m_camera;
	size_t m_visSize[2];
	friend class Light_Point_Creator;


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

class Light_Point_Creator : public ComponentCreator
{
public:
	Light_Point_Creator() : ComponentCreator() {}
	virtual Component* create(Engine * engine) {
		return new Light_Point_C(engine);
	}
};

#endif // LIGHT_POINT_COMPONENT_H