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

#include "Assets\Asset_Primitive.h"

namespace dt_ASSIMP {
}

namespace Asset_Manager {
	DELTA_ASSIMP_API void load_asset(Shared_Asset_Primitive &user, const string & filename, const bool &threaded = true);
}
#endif // DT_ASSIMP