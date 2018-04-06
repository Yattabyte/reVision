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

#include "Utilities\GL\StaticBuffer.h"
#include <vector>


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
	Uniform_Buffer(const GLuint & target = GL_UNIFORM_BUFFER) : m_target(target) {
		m_count = 0;
	}


	// Public Methods
	/** Binds this buffer to its target slot. */
	void bindBuffer() {
		m_buffer.bindBufferBase(m_target, 5);
	}
	/** Add an element to this buffers list. 
	 * @param	uboIndex	the element to add to this list */
	void * const addElement(unsigned int * uboIndex) {
		*uboIndex = m_count++;
		m_indexPointers.push_back(uboIndex);
		return m_buffer.getBufferPointer();
	}
	

	// Public Interface
	/** Remove an element from this buffers list. *
	 * @param	uboIndex	the element to remove from this list */
	virtual	void removeElement(const unsigned int * uboIndex) {	}


protected:
	// Protected Methods
	template <typename T>
	void replaceWithEnd(const unsigned int * uboIndex) {
		if (*uboIndex < m_indexPointers.size() - 1) {
			// Move the pointer from the last element of the list to the spot we are deleting
			unsigned int * lastIndex = m_indexPointers.back();
			m_indexPointers.at(*uboIndex) = lastIndex;
			m_indexPointers.pop_back();

			// Move the memory from the last index to the old index
			void * bufferPtr = m_buffer.getBufferPointer();
			T * oldData = &reinterpret_cast<T*>(bufferPtr)[*uboIndex];
			T * endData = &reinterpret_cast<T*>(bufferPtr)[*lastIndex];
			*oldData = *endData;

			// Ensure that the pointers hold the right VALUE for the index the represent
			*lastIndex = *uboIndex;
		}
		m_count--;
	}


	// Protected Attributes
	unsigned int m_count;
	GLuint m_target;
	std::vector<unsigned int *> m_indexPointers;
	StaticBuffer m_buffer;
};

#endif // UNIFORM_BUFFER