/*
	Material Manager

	- Manages the creation and storage of materials, and to a lesser degree their destruction
	- Supports bindless textures
	- How this works:
		- This stores an array of GLuint64 bindless texture handles
		- That array is stored as an shader storage buffer object (SSBO), with the ID statically stored in cpp file
		- Geometry requests a spot in the material buffer array:
			- this provides an int index
			- geometry stores it alongside vertices (MUST NOT be interpolated across face)
		- Texture accessed in fragment shader by passing in the index spot from vertex shader to fragment shader:
			- get GLuint64 from ssbo's array using index
			- transform into sampler
	- Why this is used:
		- Texture binding is slow, this circumvents it
		- Many separate materials PER model
*/

#pragma once
#ifndef MATERIAL_MANAGER
#define MATERIAL_MANAGER
#ifdef	ENGINE_EXPORT
#define DT_ENGINE_API __declspec(dllexport)
#else
#define	DT_ENGINE_API __declspec(dllimport)
#endif
#define ZERO_MEM(a) memset(a, 0, sizeof(a))
#define MAX_NUM_MAPPED_TEXTURES 500	

#include "GL\glew.h"
#include <deque>

using namespace std; 

struct Material_Buffer
{
	Material_Buffer() {
		ZERO_MEM(MaterialMaps);
	}
	GLuint64 MaterialMaps[MAX_NUM_MAPPED_TEXTURES]; // The bindless texture handles
};

namespace Material_Manager {
	// Start up and initialize the material manager
	DT_ENGINE_API void startup();
	// Shut down and flush out the material manager
	DT_ENGINE_API void shutdown();
	// Returns a list of spots within the material manager that have been deleted and can be used for new materials
	DT_ENGINE_API deque<int>& getMatFreeSpots();
	// returns the material buffer object ID for use in shaders
	DT_ENGINE_API GLuint& getBufferSSBO();
}

#endif // MATERIAL_MANAGER
