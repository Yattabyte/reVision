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
#ifdef	DT_CORE_EXPORT
#define DELTA_CORE_API __declspec(dllexport)
#else
#define	DELTA_CORE_API __declspec(dllimport)
#endif
#define ZERO_MEM(a) memset(a, 0, sizeof(a))
#define MAX_NUM_MAPPED_TEXTURES 500	

#include "GL\glew.h"
#include <deque>

using namespace std;

namespace Material_Manager {
	struct Material_Buffer
	{
		Material_Buffer() {
			ZERO_MEM(MaterialMaps);
		}
		GLuint64 MaterialMaps[MAX_NUM_MAPPED_TEXTURES]; // The bindless texture handles
	};

	DELTA_CORE_API void startup();
	DELTA_CORE_API deque<int>& getMatFreeSpots();
	DELTA_CORE_API GLuint& getBufferSSBO();
}

#endif // MATERIAL_MANAGER
