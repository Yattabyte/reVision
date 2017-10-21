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

namespace dt_ASSIMP {
}

#endif // DT_ASSIMP