#pragma once
#ifndef STATICMAPPEDBUFFER_H
#define STATICMAPPEDBUFFER_H

#include "Utilities/GL/Buffer_Interface.h"
#include <utility>


/** Encapsulates an OpenGL buffer that is fixed in size, and mapped to system memory. */
class StaticMappedBuffer : public Buffer_Interface {
public:
	// Public (de)Constructors
	inline ~StaticMappedBuffer() {
		if (m_bufferID != 0) {
			glUnmapNamedBuffer(m_bufferID);
			glDeleteBuffers(1, &m_bufferID);
		}
	}
	/** Default Constructor. */
	inline StaticMappedBuffer() = default;
	/** Explicit Instantion. */
	inline StaticMappedBuffer(const GLsizeiptr & size, const void * data = 0, const GLbitfield & mapFlags = GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT) {
		glCreateBuffers(1, &m_bufferID);
		glNamedBufferStorage(m_bufferID, size, data, GL_DYNAMIC_STORAGE_BIT | mapFlags);
		m_bufferPtr = glMapNamedBufferRange(m_bufferID, 0, size, mapFlags);
	}
	/** Explicit Instantion. */
	inline StaticMappedBuffer(StaticMappedBuffer && other) : m_bufferID(0), m_bufferPtr(nullptr) {
		*this = std::move(other);
	}
	/** Move gl object from 1 instance to another. */
	inline StaticMappedBuffer & operator=(StaticMappedBuffer && other) {
		if (this != &other) {
			m_bufferID = other.m_bufferID;
			m_bufferPtr = other.m_bufferPtr;
			other.m_bufferID = 0;
			other.m_bufferPtr = nullptr;
		}
		return *this;
	}


	// Public Inteface Implementations
	inline virtual void bindBuffer(const GLenum & target) const override {
		glBindBuffer(target, m_bufferID);
	}
	inline virtual void bindBufferBase(const GLenum & target, const GLuint & index) const override {
		glBindBufferBase(target, index, m_bufferID);
	}
		

	// Public Methods
	/** Write the supplied data to GPU memory
	@param	offset	byte offset from the beginning
	@param	size	the size of the data to write
	@param	data	the data to write */
	inline void write(const GLsizeiptr & offset, const GLsizeiptr & size, const void * data) {
		std::memcpy(reinterpret_cast<unsigned char*>(m_bufferPtr) + offset, data, size);
	}
	/** Write the supplied data to GPU memory
	@param	offset	byte offset from the beginning
	@param	size	the size of the data to write
	@param	data	the data to write */
	inline void write_immediate(const GLuint & offset, const GLsizeiptr & size, const void * data) {
		glNamedBufferSubData(m_bufferID, offset, size, data);
	}


private:
	// Private Attributes
	GLuint m_bufferID = 0;
	void * m_bufferPtr = nullptr;
};

#endif // STATICMAPPEDBUFFER_H