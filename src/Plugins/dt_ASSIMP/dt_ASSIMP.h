/*
	dt_ASSIMP : http://assimp.sourceforge.net/

	- A plugin which uses ASSIMP for loading models from disk
*/

#pragma once
#ifndef	DT_ASSIMP
#define	DT_ASSIMP
#ifdef	DT_ASSIMP_EXPORT
#define DELTA_ASSIMP_API __declspec(dllexport)
#else
#define	DELTA_ASSIMP_API __declspec(dllimport)
#endif

#define GLEW_STATIC
#include "GL\glew.h"
#include "Assets\Asset_Collider.h"
#include "Assets\Asset_Primitive.h"
#include "Managers\Asset_Manager.h"

namespace dt_ASSIMP {
	// Initializes the assimp plugin and any systems it needs
	// Returns true if it successfull, false otherwise. Reports its own errors to the console.
	DELTA_ASSIMP_API bool Initialize();
}

namespace Asset_Manager {
	DELTA_ASSIMP_API void load_asset(Shared_Asset_Collider &user, const string & filename, const bool &threaded = true);
	DELTA_ASSIMP_API void load_asset(Shared_Asset_Primitive &user, const string & filename, const bool &threaded = true);
}

#endif // DT_ASSIMP