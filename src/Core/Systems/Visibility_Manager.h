/*
	Visibility_Manager

	- Stores different types of geometry into an organized map, grouped by type into separate vectors
*/

#pragma once
#ifndef VISIBILITY_MANAGER
#define VISIBILITY_MANAGER
#ifdef	ENGINE_EXPORT
#define DELTA_CORE_API __declspec(dllexport)
#else
#define	DELTA_CORE_API __declspec(dllimport)
#endif

#include "Rendering\Camera.h"
#include "Entities\Geometry.h"
#include <mutex>
#include <shared_mutex>
#include <vector>

using namespace std;

namespace Visibility_Manager {
	DELTA_CORE_API void statup();
	DELTA_CORE_API void shutdown();
	DELTA_CORE_API void pause();
	DELTA_CORE_API void resume();
	void RegisterViewer(Camera *camera);
	void UnRegisterViewer(Camera *camera);
	void DeRegisterGeometryFromViewers(Geometry *g);	
	void Visibility_Checker();
};

#endif // VISIBILITY_MANAGER