/*
	Geometry_Manager

	- 
*/

#pragma once
#ifndef GEOMETRY_MANAGER
#define GEOMETRY_MANAGER
#ifdef	ENGINE_EXPORT
#define DT_ENGINE_API __declspec(dllexport)
#else
#define	DT_ENGINE_API __declspec(dllimport)
#endif

#include "Rendering\Visibility_Token.h"
#include "Rendering\Camera.h"

namespace Geometry_Manager {

	DT_ENGINE_API void CalcVisibility(Camera &camera);
}

#endif
