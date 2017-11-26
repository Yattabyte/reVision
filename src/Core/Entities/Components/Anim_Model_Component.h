/*
	Animated_Model_Component

	- A renderable model component
	- Supports an animated skeleton
*/

#pragma once
#ifndef ANIM_MODEL_COMPONENT
#define ANIM_MODEL_COMPONENT
#ifdef	ENGINE_EXPORT
#define DT_ENGINE_API __declspec(dllexport)
#else
#define	DT_ENGINE_API __declspec(dllimport)
#endif

#include "Entities\Components\Geometry_Component.h"
#include "Assets\Asset_Model.h"
#include "glm\glm.hpp"

using namespace glm;

struct Transform_Buffer {
	int useBones; vec3 padding1;
	mat4 mMatrix;
	mat4 transforms[NUM_MAX_BONES];
	Transform_Buffer()
	{
		useBones = 0;
		mMatrix = mat4(1.0f);
	}
};

class Anim_Model_Creator;
class Anim_Model_Component : protected Geometry_Component
{
public:
	/*************
	----Common----
	*************/

	// Logic for interpreting receiving messages
	DT_ENGINE_API virtual void ReceiveMessage(const ECSmessage &message);


	/***************************
	----Anim_Model Functions----
	***************************/

	// Renders the model to the current framebuffer
	DT_ENGINE_API virtual void Draw();
	// Returns whether or not this model is visible
	DT_ENGINE_API virtual bool IsVisible(const mat4 & PVMatrix);
	// Sends current data to the GPU
	DT_ENGINE_API void Update();


protected:
	DT_ENGINE_API ~Anim_Model_Component();
	DT_ENGINE_API Anim_Model_Component(const ECShandle &id, const ECShandle &pid);
	friend class Anim_Model_Creator;
	
	GLuint m_uboID;
	Transform_Buffer m_uboData;
	Shared_Asset_Model m_model;
};

class DT_ENGINE_API Anim_Model_Creator : public ComponentCreator
{
	virtual Component* Create(const ECShandle &id, const ECShandle &pid) {
		return new Anim_Model_Component(id, pid);
	}
};

#endif // ANIM_MODEL_COMPONENT