/*
	Visibility_Manager

	- Stores different types of geometry into an organized map, grouped by type into separate vectors
*/

#pragma once
#ifndef VISIBILITY_MANAGER
#define VISIBILITY_MANAGER
#ifdef	ENGINE_EXPORT
#define DT_ENGINE_API __declspec(dllexport)
#else
#define	DT_ENGINE_API __declspec(dllimport)
#endif

#include "Rendering\Camera.h"
#include "Entities\Geometry.h"
#include <mutex>
#include <shared_mutex>
#include <vector>

using namespace std;

namespace Visibility_Manager {
	DT_ENGINE_API void statup();
	DT_ENGINE_API void shutdown();
	DT_ENGINE_API void pause();
	DT_ENGINE_API void resume();
	void RegisterViewer(Camera *camera);
	void UnRegisterViewer(Camera *camera);
	void DeRegisterGeometryFromViewers(Geometry *g);	
	void Visibility_Checker();
};

#endif // VISIBILITY_MANAGER