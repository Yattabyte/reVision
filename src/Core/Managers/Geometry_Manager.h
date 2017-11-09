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

using namespace std;

namespace Geometry_Manager {
	DELTA_CORE_API void registerGeometry(const int &typeID, Geometry *geometry);
	DELTA_CORE_API void unregisterGeometry(const int &typeID, Geometry *geometry);

	// TO DO:
	//void Update_VisibilityToken() {};
	//void GeometryPass() {};

}

#endif // GEOMETRY_MANAGER