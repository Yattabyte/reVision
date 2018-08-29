#pragma once
#ifndef STATICBUFFER_H
#define STATICBUFFER_H

#include "GL\glew.h"
#include <utility>


/** Encapsulates an OpenGL buffer that is fixed in size. */
class StaticBuffer {
public:
	// Public (de)Constructors
	~StaticBuffer() {
		if (m_bufferID != 0) {
			glDeleteBuffers(1, &m_bufferID);
		}
	}
	/** Default Constructor. */
	StaticBuffer() = default;
	/** Explicit Instantion. */
	StaticBuffer(const GLsizeiptr & size, const void * data = 0) {
		glCreateBuffers(1, &m_bufferID);
		glNamedBufferStorage(m_bufferID, size, data, GL_DYNAMIC_STORAGE_BIT);
	}
	/** Explicit Instantion. */
	StaticBuffer(StaticBuffer && other) : m_bufferID(0) {
		*this = std::move(other);
	}
	/** Move gl object from 1 instance to another. */
	StaticBuffer & operator=(StaticBuffer && other) {
		if (this != &other) {
			m_bufferID = other.m_bufferID;
			other.m_bufferID = 0;
		}
		return *this;
	}
		

	// Public Methods
	/** Bind this buffer.
	@param	target	the target type of this buffer */
	void bindBuffer(const GLenum & target) const {
		glBindBuffer(target, m_bufferID);
	}
	/** Bind this buffer to a particular binding point for shaders.
	@param	target	the target type of this buffer
	@param	index	the binding point index to use */
	void bindBufferBase(const GLenum & target, const GLuint & index) const {
		glBindBufferBase(target, index, m_bufferID);
	}
	/** Write the supplied data to GPU memory
	@param	offset	byte offset from the beginning
	@param	size	the size of the data to write
	@param	data	the data to write */
	void write(const GLsizeiptr & offset, const GLsizeiptr & size, const void * data) {
		glNamedBufferSubData(m_bufferID, offset, size, data);
	}


private:
	// Private Attributes
	GLuint m_bufferID = 0;
};

#endif // STATICBUFFER_H