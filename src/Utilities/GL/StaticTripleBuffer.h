#pragma once
#ifndef STATICTRIPLEBUFFER_H
#define STATICTRIPLEBUFFER_H

#include "Utilities/GL/Buffer_Interface.h"
#include <utility>


/** Encapsulates an OpenGL buffer that is fixed in size. */
class StaticTripleBuffer : public Buffer_Interface {
public:
	// Public (de)Constructors
	inline ~StaticTripleBuffer() {
		glUnmapNamedBuffer(m_bufferID[0]);
		glUnmapNamedBuffer(m_bufferID[1]);
		glUnmapNamedBuffer(m_bufferID[2]);
		glDeleteBuffers(3, m_bufferID);
	}
	/** Default Constructor. */
	inline StaticTripleBuffer() = default;
	/** Explicit Instantion. */
	inline StaticTripleBuffer(const GLsizeiptr & size, const void * data = 0, const GLbitfield & storageFlags = GL_DYNAMIC_STORAGE_BIT)
		: m_size(size) {
		glCreateBuffers(3, m_bufferID);
		for (int x = 0; x < 3; ++x) {
			glNamedBufferStorage(m_bufferID[x], m_size, data, storageFlags | m_mapFlags);
			m_bufferPtr[x] = glMapNamedBufferRange(m_bufferID[x], 0, m_size, m_mapFlags);
			if (data)
				std::memcpy(reinterpret_cast<unsigned char*>(m_bufferPtr[x]), data, size);
		}
	}
	/** Explicit Instantion. */
	inline StaticTripleBuffer(const StaticTripleBuffer & other)
		: StaticTripleBuffer(other.m_size, 0) {
		for (int x = 0; x < 3; ++x)
			glCopyNamedBufferSubData(other.m_bufferID[x], m_bufferID[x], 0, 0, m_size);
	}
	/** Explicit Instantion. */
	inline StaticTripleBuffer(StaticTripleBuffer && other) {
		*this = std::move(other);
	}
	/** Move gl object from 1 instance to another. */
	inline StaticTripleBuffer & operator=(StaticTripleBuffer && o) {
		m_bufferID[0] = (std::move(o.m_bufferID[0]));
		m_bufferID[1] = (std::move(o.m_bufferID[1]));
		m_bufferID[2] = (std::move(o.m_bufferID[2]));
		m_bufferPtr[0] = (std::move(o.m_bufferPtr[0]));
		m_bufferPtr[1] = (std::move(o.m_bufferPtr[1]));
		m_bufferPtr[2] = (std::move(o.m_bufferPtr[2]));
		m_size = (std::move(o.m_size));
		o.m_bufferID[0] = 0;
		o.m_bufferID[1] = 0;
		o.m_bufferID[2] = 0;
		o.m_bufferPtr[0] = nullptr;
		o.m_bufferPtr[1] = nullptr;
		o.m_bufferPtr[2] = nullptr;
		o.m_size = 0;
		return *this;
	}


	// Public Inteface Implementations
	inline virtual void bindBuffer(const GLenum & target) const override {
		glBindBuffer(target, m_bufferID[m_writeIndex]);
	}
	inline virtual void bindBufferBase(const GLenum & target, const GLuint & index) const override {
		glBindBufferBase(target, index, m_bufferID[m_writeIndex]);
	}
		

	// Public Methods
	/** Write the supplied data to GPU memory
	@param	offset	byte offset from the beginning
	@param	size	the size of the data to write
	@param	data	the data to write */
	inline void write(const GLsizeiptr & offset, const GLsizeiptr & size, const void * data) {
		std::memcpy(reinterpret_cast<unsigned char*>(m_bufferPtr[m_writeIndex]) + offset, data, size);
	}
	/** Wait for the current fence to pass, for the current frame index. */
	inline void beginWriting() {
		if (m_fence[m_writeIndex] != nullptr)
			while (1) {
				GLenum waitReturn = glClientWaitSync(m_fence[m_writeIndex], GL_SYNC_FLUSH_COMMANDS_BIT, 1);
				if (waitReturn == GL_SIGNALED || waitReturn == GL_ALREADY_SIGNALED || waitReturn == GL_CONDITION_SATISFIED) {
					glDeleteSync(m_fence[m_writeIndex]);
					m_fence[m_writeIndex] = nullptr;
					return;
				}
			}
	}
	/** Create a fence for the current point in time. */
	inline void endWriting() {
		if (!m_fence[m_writeIndex]) {
			m_fence[m_writeIndex] = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
			m_writeIndex = (m_writeIndex + 1) % 3;
		}
	}


private:
	// Private Attributes
	int m_writeIndex = 0;
	GLuint m_bufferID[3] = { 0,0,0 };
	void * m_bufferPtr[3] = { nullptr, nullptr, nullptr };
	GLsync m_fence[3] = { nullptr, nullptr, nullptr };
	size_t m_size = 0ull;
	GLbitfield m_mapFlags = GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;
};

#endif // STATICTRIPLEBUFFER_H