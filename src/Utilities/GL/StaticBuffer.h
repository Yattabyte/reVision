#pragma once
#ifndef STATICBUFFER_H
#define STATICBUFFER_H
#ifdef	ENGINE_EXE_EXPORT
#define DT_ENGINE_API 
#elif	ENGINE_DLL_EXPORT 
#define DT_ENGINE_API __declspec(dllexport)
#else
#define	DT_ENGINE_API __declspec(dllimport)
#endif

#include "GL\glew.h"
#include <utility>


class StaticBuffer
{
public:
	// Public (de)Constructors
	~StaticBuffer() {
		if (m_bufferID != 0) {
			glUnmapNamedBuffer(m_bufferID);
			glDeleteBuffers(1, &m_bufferID);
		}
	}
	/** Default. */
	StaticBuffer() {
		m_bufferID = 0;
		m_bufferPtr = nullptr;
	}
	/** Explicit Instantion. */
	StaticBuffer(const GLsizeiptr & size, const void * data, const GLbitfield & mapFlags = GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT) : StaticBuffer() {
		glCreateBuffers(1, &m_bufferID);
		glNamedBufferStorage(m_bufferID, size, data, GL_DYNAMIC_STORAGE_BIT | mapFlags);
		m_bufferPtr = glMapNamedBufferRange(m_bufferID, 0, size, mapFlags);
	}
	/** Move gl object from 1 instance to another. */
	StaticBuffer & operator=(StaticBuffer && o) noexcept {
		m_bufferID = (std::move(o.m_bufferID));
		m_bufferPtr = (std::move(o.m_bufferPtr));
		o.m_bufferID = 0;
		o.m_bufferPtr = nullptr;
		return *this;
	}
		

	// Public Methods
	/** Bind this buffer.
	* @param	target	the target type of this buffer */
	void bindBuffer(const GLenum & target) const {
		glBindBuffer(target, m_bufferID);
	}
	/** Bind this buffer to a particular binding point for shaders.
	 * @param	target	the target type of this buffer
	 * @param	index	the binding point index to use */
	void bindBufferBase(const GLenum & target, const GLuint & index) const {
		glBindBufferBase(target, index, m_bufferID);
	}
	/** Cast this buffer's pointer to a type, as to allow modifying its underlying data. 
	 * @return			the pointer to this data in memory, cast to the type specified
	 * @param	<T>		the type to cast this to */
	template <typename T>
	T castPointer() {
		return reinterpret_cast<T>(m_bufferPtr);
	}
	/** Write the supplied data to GPU memory
	 * @param	offset	byte offset from the beginning
	 * @param	size	the size of the data to write
	 * @param	data	the data to write */
	void write(const GLsizeiptr & offset, const GLsizeiptr & size, const void * data) {
		std::memcpy(reinterpret_cast<unsigned char*>(m_bufferPtr) + offset, data, size);
	}
	/** Write the supplied data to GPU memory
	* @param	offset	byte offset from the beginning
	* @param	size	the size of the data to write
	* @param	data	the data to write */
	void write_immediate(const GLuint & offset, const GLsizeiptr & size, const void * data) {
		glNamedBufferSubData(m_bufferID, offset, size, data);
	}
	/** Retrieve the mapped buffer pointer. */
	void * getBufferPointer() const {
		return m_bufferPtr;
	}


private:
	// Private Attributes
	GLuint m_bufferID;
	void * m_bufferPtr;
};

#endif // STATICBUFFER_H