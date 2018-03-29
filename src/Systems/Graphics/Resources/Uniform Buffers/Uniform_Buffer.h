#pragma once
#ifndef UNIFORM_BUFFER
#define UNIFORM_BUFFER
#ifdef	ENGINE_EXE_EXPORT
#define DT_ENGINE_API 
#elif	ENGINE_DLL_EXPORT 
#define DT_ENGINE_API __declspec(dllexport)
#else
#define	DT_ENGINE_API __declspec(dllimport)
#endif
#define GLEW_STATIC

#include "Utilities\GL\MappedBuffer.h"


/**
 * A wrapper class for GL uniform buffers that hold an array of data
 */
class DT_ENGINE_API Uniform_Buffer
{
public:
	// (de)Constructors
	/** Destroy the uniform buffer. */
	~Uniform_Buffer() {

	}
	/** Construct the uniform buffer. */
	Uniform_Buffer() {
		m_count = 0;
	}


	// Public Methods
	void bindBuffer() {
		m_buffer.bindBufferBase(GL_UNIFORM_BUFFER, 5);
	}


protected:
	// Protected Methods
	void * const addElement(unsigned int & uboIndex) {
		uboIndex = m_count++;
		return m_buffer.getBufferPointer();
	}
	void removeElement(const unsigned int & uboIndex) {

	}


	// Protected Attributes
	unsigned int m_count;
	MappedBuffer m_buffer;
};

#endif // UNIFORM_BUFFER