/*
	Lighting_Component

	- A base class for lighting components
	- Does nothing on its own, just exposes common methods for other components
*/

#pragma once
#ifndef LIGHTING_COMPONENT
#define LIGHTING_COMPONENT
#ifdef	ENGINE_EXPORT
#define DT_ENGINE_API __declspec(dllexport)
#else
#define	DT_ENGINE_API __declspec(dllimport)
#endif

#include "Entities\Components\Component.h"
#include "Systems\World\Visibility_Token.h"
#include "glm\glm.hpp"

using namespace glm;

class DT_ENGINE_API Lighting_Component : protected Component
{
public:

	/*************************
	----Lighting Functions----
	*************************/

	// Direct lighting pass
	virtual void directPass(const int &vertex_count) {};
	// Indirect lighting pass
	virtual void indirectPass(const int &vertex_count) {};
	// Shadow lighting pass
	virtual void shadowPass() {};
	// Returns whether or not this light is visible
	virtual bool IsVisible(const mat4 & PMatrix, const mat4 &VMatrix) { return false; };
	// Returns the timestamp of the last time this light updated its shadowmap
	double getShadowUpdateTime() const { return m_shadowUpdateTime; }


protected:
	~Lighting_Component() {};
	Lighting_Component(const ECShandle &id, const ECShandle &pid) : Component(id, pid) { m_shadowUpdateTime = 0; };
	double m_shadowUpdateTime;
};

#endif // LIGHTING_COMPONENT