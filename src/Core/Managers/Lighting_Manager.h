/*
	Lighting_Manager

	- Stores different types of lights into an organized map, grouped by type into separate vectors
*/

#pragma once
#ifndef LIGHTING_MANAGER
#define LIGHTING_MANAGER
#ifdef	DT_CORE_EXPORT
#define DELTA_CORE_API __declspec(dllexport)
#else
#define	DELTA_CORE_API __declspec(dllimport)
#endif

#include "Entities\Light.h"
#include <map>
#include <mutex>
#include <shared_mutex>
#include <vector>

using namespace std;

namespace Lighting_Manager {
	// Using the class type ID @typeID as a grouping key, registers and groups the given light @light
	DELTA_CORE_API void RegisterLight(const int &typeID, Light *light);
	// Searches the register at @typeID and removes all instances of @light
	DELTA_CORE_API void UnRegisterLight(const int &typeID, Light *light);
	// Returns this system's shared mutex
	DELTA_CORE_API shared_mutex &GetDataLock();
	// Returns the std::map containing all of the stored light data
	// WARNING: Make sure to lock the data to prevent corruption and or crashes!
	DELTA_CORE_API map<int, vector<Light*>> &GetAllLights();

	// TO DO:
	//void Update_VisibilityToken() {};
}

#endif // LIGHTING_MANAGER