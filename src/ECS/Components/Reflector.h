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
class Reflector_C : public Component
{
public:
	// Interface implementations
	static const char * GetName() { return "Reflector"; }
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
	/** Construct by means of an argument list. */
	Reflector_C(Engine * engine, const ArgumentList & argumentList);;
	/** Constructors a reflector component. 
	 * @param	engine		the engine to use
	 * @param	transform	the transform to use */
	Reflector_C(Engine * engine, const Transform & transform = Transform());


	// Protected Attributes
	unsigned int m_uboIndex;
	VB_Ptr * m_uboBuffer;
	glm::vec3 m_position;
	glm::vec3 m_scale;
	Camera m_cameras[6];
	friend class ECS;
	friend class Component_Creator<Reflector_C>;


private:
	// Private Functions
	/** Set the transformation for this component.
	 * @param	transform	the transform to use */
	void setTransform(const Transform & transform);
};

#endif // REFLECTOR_COMPONENT_H