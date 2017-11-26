/*
	Shadowmap_Manager

	- Manages the creation and storage of shadowmaps for lights
	- Kinda like a buffer but also like a manager
	- Is static, so only one version of this exists, but has its own frame buffers
*/

#pragma once
#ifndef SHADOWMAP_MANAGER
#define SHADOWMAP_MANAGER
#ifdef	ENGINE_EXPORT
#define DT_ENGINE_API __declspec(dllexport)
#else
#define	DT_ENGINE_API __declspec(dllimport)
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
	DT_ENGINE_API void startup();
	// Clears everything out of the shadowmap manager
	DT_ENGINE_API void shutdown();
	// 
	DT_ENGINE_API void RegisterShadowCaster(const int & shadow_type, int & array_spot);
	// 
	DT_ENGINE_API void UnRegisterShadowCaster(const int & shadow_type, int & array_spot);
	// 
	DT_ENGINE_API void BindForWriting(const int & ShadowSpot);
	// 
	DT_ENGINE_API void BindForReading(const int & ShadowSpot, const GLuint & ShaderTextureUnit);
}

#endif // SHADOWMAP_MANAGER