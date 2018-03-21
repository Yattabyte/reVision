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
	virtual void draw();
	virtual bool isVisible(const mat4 & PMatrix, const mat4 &VMatrix);


	// Public Methods
	/** Sends current data to the GPU. */
	void update();


protected:
	/** Nested Buffer class.
	 * @brief	used for sending data to the gpu. */
	struct Transform_Buffer {	
		mat4 mMatrix;
		vec4 BoxCamPos;
		int CubeSpot;
		int Stencil;
		vec2 padding;
		Transform_Buffer() {
			mMatrix = mat4(1.0f);
			BoxCamPos = vec4(0.0f);
			CubeSpot = 0;
			Stencil = 0;
		}
	};


	// (de)Constructors
	/** Destroys a reflector component. */
	~Reflector_Component();
	/** Constructors a reflector component. */
	Reflector_Component(const ECShandle &id, const ECShandle &pid, EnginePackage *enginePackage);


	// Protected Attributes
	unsigned int m_uboIndex;
	void * m_uboBuffer;
	Transform_Buffer m_uboData;
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