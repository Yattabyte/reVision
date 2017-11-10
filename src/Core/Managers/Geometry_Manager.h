/*
	Geometry_Manager

	- 
*/

#pragma once
#ifndef GEOMETRY_MANAGER
#define GEOMETRY_MANAGER
#ifdef	DT_CORE_EXPORT
#define DELTA_CORE_API __declspec(dllexport)
#else
#define	DELTA_CORE_API __declspec(dllimport)
#endif

#include "Entities\Geometry.h"
#include <map>
#include <mutex>
#include <shared_mutex>
#include <vector>

using namespace std;

namespace Geometry_Manager {
	// Using the class type ID @typeID as a grouping key, registers and groups the given geometry @geometry
	DELTA_CORE_API void RegisterGeometry(const int &typeID, Geometry *geometry);
	// Searches the register at @typeID and removes all instances of @geometry
	DELTA_CORE_API void UnRegisterGeometry(const int &typeID, Geometry *geometry);
	// Returns this system's shared mutex
	DELTA_CORE_API shared_mutex &GetDataLock();
	// Returns the std::map containing all of the stored geometry data
	// WARNING: Make sure to lock the data to prevent corruption and or crashes!
	DELTA_CORE_API map<int, vector<Geometry*>> &GetAllGeometry();

	// TO DO:
	//void Update_VisibilityToken() {};
	//void GeometryPass() {};

}

#endif // GEOMETRY_MANAGER