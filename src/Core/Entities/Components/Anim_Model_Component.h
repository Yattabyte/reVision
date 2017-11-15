/*
	Animated_Model_Component

	- A renderable model component
	- Supports an animated skeleton
*/

#pragma once
#ifndef ANIM_MODEL_COMPONENT
#define ANIM_MODEL_COMPONENT
#ifdef	DT_CORE_EXPORT
#define DELTA_CORE_API __declspec(dllexport)
#else
#define	DELTA_CORE_API __declspec(dllimport)
#endif
#define TYPE_COMPONENT_ANIM_MODEL unsigned int(0)

#include "Entities\Components\Component.h"
#include "Assets\Asset_Model.h"
#include "Utilities\Transform.h"
#include "glm\glm.hpp"
#include <string>

using namespace glm;

struct Transform_Buffer {
	int useBones; vec3 padding1;
	mat4 mMatrix;
	mat4 transforms[NUM_MAX_BONES];
};

class Anim_Model_Component : public Component
{
public:
	DELTA_CORE_API ~Anim_Model_Component();
	DELTA_CORE_API Anim_Model_Component(const string & relativePath, Transform *worldState);	
	DELTA_CORE_API void Update();
	DELTA_CORE_API void Draw();
	virtual unsigned int GetTypeID() { return TYPE_COMPONENT_ANIM_MODEL; }


	GLuint m_uboID;
	Transform_Buffer m_uboData;
	Transform * m_transformData;
	Shared_Asset_Model m_model;
};

#endif // TEST_COMPONENT