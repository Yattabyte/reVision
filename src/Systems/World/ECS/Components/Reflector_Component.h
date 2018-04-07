#pragma once
#ifndef REFLECTOR_COMPONENT
#define REFLECTOR_COMPONENT
#ifdef	ENGINE_EXE_EXPORT
#define DT_ENGINE_API 
#elif	ENGINE_DLL_EXPORT 
#define DT_ENGINE_API __declspec(dllexport)
#else
#define	DT_ENGINE_API __declspec(dllimport)
#endif
#define GLM_FORCE_SWIZZLE
#define GLM_SWIZZLE_XYZ

#include "Systems\World\ECS\Components\Component.h"
#include "Utilities\GL\VectorBuffer.h"
#include "GL\glew.h"
#include "glm\glm.hpp"

using namespace glm;
class Reflector_Creator;
class EnginePackage;


/**
 * An object that records the world around it and uses it to project reflections.
 **/
class DT_ENGINE_API Reflector_Component : protected Component
{
public:
	// Interface implementations
	virtual void receiveMessage(const ECSmessage &message);


	// Public Methods
	/** Retrieve the buffer index for this reflector.
	 * @return	the buffer index */
	const unsigned int getBufferIndex() const;
	/** Tests if this object is within the viewing frustum of the camera.
	 * @brief				a test of general visibility (excluding obstruction of other objects).
	 * @param	PMatrix		the projection matrix of the camera
	 * @param	VMatrix		the viewing matrix of the camera
 	 * @return				true if this object is within the viewing frustum of the camera, false otherwise */
	virtual bool isVisible(const mat4 & PMatrix, const mat4 &VMatrix) const;
	

protected:
	// (de)Constructors
	/** Destroys a reflector component. */
	~Reflector_Component();
	/** Constructors a reflector component. */
	Reflector_Component(const ECShandle &id, const ECShandle &pid, EnginePackage *enginePackage);


	// Protected Attributes
	unsigned int m_uboIndex;
	VB_Ptr * m_uboBuffer;
	vec3 m_position;
	vec3 m_scale;
	GLsync m_fence; 
	EnginePackage *m_enginePackage;
	friend class Reflector_Creator;
};

class DT_ENGINE_API Reflector_Creator : public ComponentCreator
{
public:
	Reflector_Creator(ECSmessenger *ecsMessenger) : ComponentCreator(ecsMessenger) {}
	virtual Component* Create(const ECShandle &id, const ECShandle &pid, EnginePackage *enginePackage) {
		return new Reflector_Component(id, pid, enginePackage);
	}
};

#endif // REFLECTOR_COMPONENT