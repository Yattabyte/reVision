/*
	Geometry_Component

	- A base class for renderable components
	- Does nothing on its own, just exposes common methods for other components
*/

#pragma once
#ifndef GEOMETRY_COMPONENT
#define GEOMETRY_COMPONENT
#ifdef	DT_CORE_EXPORT
#define DELTA_CORE_API __declspec(dllexport)
#else
#define	DELTA_CORE_API __declspec(dllimport)
#endif

#include "Entities\Components\Component.h"
#include "glm\glm.hpp"

using namespace glm;

class Geometry_Component : protected Component
{
public:

	/*************************
	----Geometry Functions----
	*************************/

	// Renders the model to the current framebuffer
	DELTA_CORE_API virtual void Draw() {};
	// Returns whether or not this model is visible
	DELTA_CORE_API virtual bool IsVisible(const mat4 & PVMatrix) { return false; };


protected:
	DELTA_CORE_API ~Geometry_Component() {};
	DELTA_CORE_API Geometry_Component(const ECSHandle &id, const ECSHandle &pid) : Component(id, pid) {};
};

#endif // GEOMETRY_COMPONENT