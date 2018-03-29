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
#define GLEW_STATIC

#include "Systems\Graphics\Resources\Uniform Buffers\Uniform_Buffer.h"


/**
* The uniform struct used by the uniform buffer
*/
struct Reflection_Struct {
	mat4 mMatrix = mat4(1.0f);
	vec4 BoxCamPos = vec4(0.0f);
	int CubeSpot = 0;
	vec3 padding;
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
		m_buffer = MappedBuffer(sizeof(Reflection_Struct) * 256, 0);
	}


	// Public Methods
	void * const addReflector(unsigned int & uboIndex) {
		return addElement(uboIndex);
	}
};

#endif // REFLECTION_UBO