/*
	Geometry_Component

	- A base class for renderable components
	- Does nothing on its own, just exposes common methods for other components
*/

#pragma once
#ifndef GEOMETRY_COMPONENT
#define GEOMETRY_COMPONENT
#ifdef	ENGINE_EXPORT
#define DT_ENGINE_API __declspec(dllexport)
#else
#define	DT_ENGINE_API __declspec(dllimport)
#endif

#include "Entities\Components\Component.h"
#include "glm\glm.hpp"

using namespace glm;

class DT_ENGINE_API Geometry_Component : protected Component
{
public:

	/*************************
	----Geometry Functions----
	*************************/

	// Renders the model to the current framebuffer
	virtual void Draw() {};
	// Returns whether or not this model is visible
	virtual bool IsVisible(const mat4 & PMatrix, const mat4 &VMatrix) { return false; };


protected:
	~Geometry_Component() {};
	Geometry_Component(const ECShandle &id, const ECShandle &pid) : Component(id, pid) {};
};

#endif // GEOMETRY_COMPONENT