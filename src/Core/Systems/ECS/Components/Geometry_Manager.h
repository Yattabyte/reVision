/*
	Geometry_Manager

	- 
*/

#pragma once
#ifndef GEOMETRY_MANAGER
#define GEOMETRY_MANAGER
#ifdef	ENGINE_EXPORT
#define DELTA_CORE_API __declspec(dllexport)
#else
#define	DELTA_CORE_API __declspec(dllimport)
#endif

#include "Rendering\Visibility_Token.h"
#include "Rendering\Camera.h"

namespace Geometry_Manager {

	DELTA_CORE_API void CalcVisibility(Camera &camera);
}

#endif
