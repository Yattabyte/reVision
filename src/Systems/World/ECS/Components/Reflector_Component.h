#pragma once
#ifndef REFLECTOR_COMPONENT_H
#define REFLECTOR_COMPONENT_H
#define GLM_FORCE_SWIZZLE
#define GLM_SWIZZLE_XYZ

#include "Systems\World\ECS\Components\Component.h"
#include "Systems\World\Camera.h"
#include "Utilities\GL\VectorBuffer.h"
#include "Utilities\Transform.h"
#include "GL\glew.h"

class Reflector_Creator;
class EnginePackage;


/**
 * An object that records the world around it and uses it to project reflections.
 **/
class Reflector_Component : protected Component
{
public:
	// Interface implementations
	virtual const char * getName() const { return "Reflector"; }
	virtual bool isVisible(const float & radius, const vec3 & eyePosition) const;


	// Public Methods
	/** Retrieve the buffer index for this reflector.
	 * @return	the buffer index */
	const unsigned int getBufferIndex() const;
	void bindCamera(const unsigned int & index) const;

protected:
	// (de)Constructors
	/** Destroys a reflector component. */
	~Reflector_Component();
	/** Constructors a reflector component. */
	Reflector_Component(EnginePackage *enginePackage);


	// Protected Attributes
	unsigned int m_uboIndex;
	VB_Ptr * m_uboBuffer;
	float m_radius;
	vec3 m_position;
	vec3 m_scale;
	Camera m_cameras[6];
	EnginePackage *m_enginePackage;
	friend class Reflector_Creator;


private:
	// Private Functions
	/** Set the reflector radius to use.
	 * @param	radius		the radius to use */
	void setRadius(const float & radius);
	/** Set the transformation for this component.
	 * @param	transform	the transform to use */
	void setTransform(const Transform & transform);
};

class Reflector_Creator : public ComponentCreator
{
public:
	Reflector_Creator() : ComponentCreator() {}
	virtual Component* create(EnginePackage *enginePackage) {
		return new Reflector_Component(enginePackage);
	}
};

#endif // REFLECTOR_COMPONENT_H