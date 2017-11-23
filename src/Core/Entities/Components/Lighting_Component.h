/*
	Lighting_Component

	- A base class for lighting components
	- Does nothing on its own, just exposes common methods for other components
*/

#pragma once
#ifndef LIGHTING_COMPONENT
#define LIGHTING_COMPONENT
#ifdef	DT_CORE_EXPORT
#define DELTA_CORE_API __declspec(dllexport)
#else
#define	DELTA_CORE_API __declspec(dllimport)
#endif

#include "Entities\Components\Component.h"
#include "Rendering\Visibility_Token.h"
#include "glm\glm.hpp"

using namespace glm;

class Lighting_Component : protected Component
{
public:

	/*************************
	----Lighting Functions----
	*************************/

	// Direct lighting pass
	DELTA_CORE_API virtual void directPass(const int &vertex_count) {};
	// Indirect lighting pass
	DELTA_CORE_API virtual void indirectPass(const int &vertex_count) {};
	// Shadow lighting pass
	DELTA_CORE_API virtual void shadowPass(const Visibility_Token &vis_token) const {};
	// Returns whether or not this light is visible
	DELTA_CORE_API virtual bool IsVisible(const mat4 & PVMatrix) { return false; };
		

protected:
	DELTA_CORE_API ~Lighting_Component() {};
	DELTA_CORE_API Lighting_Component(const ECShandle &id, const ECShandle &pid) : Component(id, pid) {};
};

#endif // LIGHTING_COMPONENT