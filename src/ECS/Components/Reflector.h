#pragma once
#ifndef REFLECTOR_COMPONENT_H
#define REFLECTOR_COMPONENT_H
#define GLM_FORCE_SWIZZLE
#define GLM_SWIZZLE_XYZ

#include "ECS\Components\Component.h"
#include "Systems\World\Camera.h"
#include "Utilities\GL\VectorBuffer.h"
#include "Utilities\Transform.h"
#include "GL\glew.h"


/**
 * An object that records the world around it and uses it to project reflections.
 **/
class Reflector_C : protected Component
{
public:
	// Interface implementations
	virtual const char * getName() const { return "Reflector"; }
	virtual bool isVisible(const float & radius, const glm::vec3 & eyePosition) const;


	// Public Methods
	/** Retrieve the buffer index for this reflector.
	 * @return	the buffer index */
	const unsigned int getBufferIndex() const;
	void bindCamera(const unsigned int & index) const;


protected:
	// (de)Constructors
	/** Destroys a reflector component. */
	~Reflector_C();
	/** Constructors a reflector component. */
	Reflector_C(Engine * engine, const Transform & transform = Transform());


	// Protected Attributes
	unsigned int m_uboIndex;
	VB_Ptr * m_uboBuffer;
	glm::vec3 m_position;
	glm::vec3 m_scale;
	Camera m_cameras[6];
	Engine *m_engine;
	friend class Component_Creator<Reflector_C>;


private:
	// Private Functions
	/** Set the transformation for this component.
	 * @param	transform	the transform to use */
	void setTransform(const Transform & transform);
};

#endif // REFLECTOR_COMPONENT_H