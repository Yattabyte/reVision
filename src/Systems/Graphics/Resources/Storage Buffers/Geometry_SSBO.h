#pragma once
#ifndef GEOMETRY_SSBO
#define GEOMETRY_SSBO
#ifdef	ENGINE_EXE_EXPORT
#define DT_ENGINE_API 
#elif	ENGINE_DLL_EXPORT 
#define DT_ENGINE_API __declspec(dllexport)
#else
#define	DT_ENGINE_API __declspec(dllimport)
#endif

#include "Systems\Graphics\Resources\Storage Buffers\Uniform_Buffer.h"
#include "Assets\Asset_Model.h"
#include "glm\glm.hpp"
using namespace glm;


/**
* The uniform struct used by the geometry SSBO
*/
struct Geometry_Struct {
	int useBones = 0;  // no padding here;
	GLuint materialID = 0; vec2 padding1; // for some reason padding here
	mat4 mMatrix = mat4(1.0f);
	mat4 transforms[NUM_MAX_BONES];
};

/**
* A uniform buffer that holds uniform data for all reflectors
**/
class DT_ENGINE_API Geometry_SSBO : public Uniform_Buffer
{
public:
	// (de)Constructors
	/** Destroy the geometry buffer. */
	~Geometry_SSBO() {}
	/** Construct the geometry buffer. */
	Geometry_SSBO() {
		m_buffer = MappedBuffer(sizeof(Geometry_Struct) * 256, 0, GL_MAP_WRITE_BIT | GL_MAP_READ_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
	}

	virtual	void removeElement(const unsigned int * uboIndex) {
		replaceWithEnd<Geometry_Struct>(uboIndex);
	}
};

#endif // GEOMETRY_SSBO