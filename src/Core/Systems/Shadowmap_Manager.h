/*
	Shadowmap_Manager

	- Manages the creation and storage of shadowmaps for lights
	- Kinda like a buffer but also like a manager
	- Is static, so only one version of this exists, but has its own frame buffers
*/

#pragma once
#ifndef SHADOWMAP_MANAGER
#define SHADOWMAP_MANAGER
#ifdef	DT_CORE_EXPORT
#define DELTA_CORE_API __declspec(dllexport)
#else
#define	DELTA_CORE_API __declspec(dllimport)
#endif
#define SHADOW_REGULAR 0 
#define SHADOW_LARGE 1
#define SHADOW_MAX 2

#include "GL\glew.h"
#include "glm\glm.hpp"

using namespace std;
using namespace glm;

namespace Shadowmap_Manager {
	// Initializes and starts up the shadowmap manager
	DELTA_CORE_API void startup();
	// Clears everything out of the shadowmap manager
	DELTA_CORE_API void shutdown();
	// 
	DELTA_CORE_API void RegisterShadowCaster(const int & shadow_type, int & array_spot);
	// 
	DELTA_CORE_API void UnRegisterShadowCaster(const int & shadow_type, int & array_spot);
	// 
	DELTA_CORE_API void BindForWriting(const int & ShadowSpot);
	// 
	DELTA_CORE_API void BindForReading(const int & ShadowSpot, const GLuint & ShaderTextureUnit);
}

#endif // SHADOWMAP_MANAGER