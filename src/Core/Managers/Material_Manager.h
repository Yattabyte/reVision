/*
	Material Manager

	- Manages the creation and storage of materials, and to a lesser degree their destruction
	- Supports bindless textures
	- How this works:
		- This stores an array of GLuint64 bindless texture handles
		- That array is stored as a shader storage buffer object (SSBO)
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
#include <shared_mutex>

using namespace std; 

struct Material_Buffer
{
	Material_Buffer() {
		ZERO_MEM(MaterialMaps);
	}
	GLuint64 MaterialMaps[MAX_NUM_MAPPED_TEXTURES]; // The bindless texture handles
};

class DT_ENGINE_API Material_Manager {
public:
	static Material_Manager &Get() {
		static Material_Manager instance;
		return instance;
	}
	// Start up and initialize the material manager
	static void Startup() { (Get())._startup(); }
	// Shut down and flush out the material manager
	static void Shutdown() { (Get())._shutdown(); }
	// Generates a material ID
	static GLuint GenerateMaterialBufferID();
	static void UpdateHandle(const GLuint &materialBufferID, const GLuint &glTextureID);

private:
	~Material_Manager() {};
	Material_Manager();
	Material_Manager(Material_Manager const&) = delete;
	void operator=(Material_Manager const&) = delete;

	void _startup();
	void _shutdown();

	shared_mutex m_DataMutex;
	bool m_Initialized; 
	GLuint m_BufferSSBO;
	Material_Buffer m_MatBuffer;
	unsigned int m_Count;
	deque<unsigned int> m_FreeSpots;
};

#endif // MATERIAL_MANAGER
