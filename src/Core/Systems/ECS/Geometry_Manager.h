/*
	Geometry_Manager

	- Stores different types of geometry into an organized map, grouped by type into separate vectors
*/

#pragma once
#ifndef GEOMETRY_MANAGER
#define GEOMETRY_MANAGER
#ifdef	DT_CORE_EXPORT
#define DELTA_CORE_API __declspec(dllexport)
#else
#define	DELTA_CORE_API __declspec(dllimport)
#endif

#include "Entities\Components\Anim_Model_Component.h"
#include <map>
#include <mutex>
#include <shared_mutex>
#include <vector>

using namespace std;

namespace Geometry_Manager {
	// Clears everything out of the geometry manager
	DELTA_CORE_API void shutdown();
	DELTA_CORE_API void CreateGeometry(const int &typeID, Geometry *geometry);
	// TO DO
	DELTA_CORE_API void UnRegisterGeometry(const int &typeID, Geometry *geometry);
	// Returns this system's shared mutex
	DELTA_CORE_API shared_mutex &GetDataLock();
	// Returns the std::map containing all of the stored geometry data
	// WARNING: Make sure to lock the data to prevent corruption and or crashes!
	DELTA_CORE_API map<int, vector<Geometry*>> &GetAllGeometry();

	// TO DO:
	//void Update_VisibilityToken() {};
}

#endif // GEOMETRY_MANAGER