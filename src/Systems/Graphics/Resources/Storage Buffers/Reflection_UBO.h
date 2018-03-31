#pragma once
#ifndef REFLECTION_UBO
#define REFLECTION_UBO
#ifdef	ENGINE_EXE_EXPORT
#define DT_ENGINE_API 
#elif	ENGINE_DLL_EXPORT 
#define DT_ENGINE_API __declspec(dllexport)
#else
#define	DT_ENGINE_API __declspec(dllimport)
#endif

#include "Systems\Graphics\Resources\Storage Buffers\Uniform_Buffer.h"
#include "glm\glm.hpp"
using namespace glm;


/**
* The uniform struct used by the reflection UBO
*/
struct Reflection_Struct {
	mat4 mMatrix = mat4(1.0f);
	vec4 BoxCamPos = vec4(0.0f);
	float Radius = 1.0f;
	int CubeSpot = 0;
	vec2 padding;
};

/**
* A uniform buffer that holds uniform data for all reflectors
**/
class DT_ENGINE_API Reflection_UBO : public Uniform_Buffer
{
public:
	// (de)Constructors
	/** Destroy the reflection buffer. */
	~Reflection_UBO() {}
	/** Construct the reflection buffer. */
	Reflection_UBO() {
		m_buffer = MappedBuffer(sizeof(Reflection_Struct) * 256, 0, GL_MAP_WRITE_BIT | GL_MAP_READ_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
	}

	virtual	void removeElement(const unsigned int * uboIndex) {
		replaceWithEnd<Reflection_Struct>(uboIndex);
	}
};

#endif // REFLECTION_UBO