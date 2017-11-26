/*
	Lighting_Manager

	- Stores different types of lights into an organized map, grouped by type into separate vectors
*/

#pragma once
#ifndef LIGHTING_MANAGER
#define LIGHTING_MANAGER
#ifdef	ENGINE_EXPORT
#define DT_ENGINE_API __declspec(dllexport)
#else
#define	DT_ENGINE_API __declspec(dllimport)
#endif

#include "Rendering\Visibility_Token.h"
#include "Rendering\Camera.h"

using namespace std;

namespace Lighting_Manager {
	DT_ENGINE_API void CalcVisibility(Camera &camera);
}

#endif // LIGHTING_MANAGER