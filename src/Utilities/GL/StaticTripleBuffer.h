#pragma once
#ifndef STATICTRIPLEBUFFER_H
#define STATICTRIPLEBUFFER_H

#include "Utilities/GL/Buffer_Interface.h"
#include <utility>


/** Encapsulates an OpenGL buffer that is fixed in size. */
template <int BufferCount = 3>
class StaticTripleBuffer final : public Buffer_Interface {
public:
	// Public (de)Constructors
	/***/
	inline ~StaticTripleBuffer() {
		for (int x = 0; x < BufferCount; ++x) {
			WaitForFence(m_writeFence[x]);
			WaitForFence(m_readFence[x]);
			if (m_bufferID[x]) {
				glUnmapNamedBuffer(m_bufferID[x]);
				glDeleteBuffers(1, &m_bufferID[x]);
			}
		}
	}
	/** Default Constructor. */
	inline StaticTripleBuffer() {
		// Zero-initialize our starting variables
		for (int x = 0; x < BufferCount; ++x) {
			m_bufferID[x] = 0;
			m_bufferPtr[x] = nullptr;
			m_writeFence[x] = nullptr;
			m_readFence[x] = nullptr;
		}
	}
	/***/
	inline StaticTripleBuffer(const GLsizeiptr& size, const void* data = 0, const GLbitfield& storageFlags = GL_DYNAMIC_STORAGE_BIT)
		: m_size(size) {
		// Zero-initialize our starting variables
		for (int x = 0; x < BufferCount; ++x) {
			m_bufferID[x] = 0;
			m_bufferPtr[x] = nullptr;
			m_writeFence[x] = nullptr;
			m_readFence[x] = nullptr;
		}

		glCreateBuffers(BufferCount, m_bufferID);
		for (int x = 0; x < BufferCount; ++x) {
			glNamedBufferStorage(m_bufferID[x], m_size, data, storageFlags | m_mapFlags);
			m_bufferPtr[x] = glMapNamedBufferRange(m_bufferID[x], 0, m_size, m_mapFlags);
			if (data)
				std::memcpy(reinterpret_cast<unsigned char*>(m_bufferPtr[x]), data, size);
		}
	}
	/***/
	inline StaticTripleBuffer(const StaticTripleBuffer& other)
		: StaticTripleBuffer(other.m_size, 0) {
		// Zero-initialize our starting variables
		for (int x = 0; x < BufferCount; ++x) {
			m_bufferID[x] = 0;
			m_bufferPtr[x] = nullptr;
			m_writeFence[x] = nullptr;
			m_readFence[x] = nullptr;
		}

		for (int x = 0; x < BufferCount; ++x)
			glCopyNamedBufferSubData(other.m_bufferID[x], m_bufferID[x], 0, 0, m_size);
	}
	/** Explicit Instantiation. */
	inline StaticTripleBuffer(StaticTripleBuffer&& other) noexcept {
		(*this) = std::move(other);
	}


	// Public Interface Implementations
	inline virtual void bindBuffer(const GLenum& target) const override final {
		// Ensure writing has finished before reading
		WaitForFence(m_writeFence[m_readIndex]);
		glBindBuffer(target, m_bufferID[m_readIndex]);
	}
	inline virtual void bindBufferBase(const GLenum& target, const GLuint& index) const override final {
		// Ensure writing has finished before reading
		WaitForFence(m_writeFence[m_readIndex]);
		glBindBufferBase(target, index, m_bufferID[m_readIndex]);
	}


	// Public Methods
	/** Write the supplied data to GPU memory
	@param	offset	byte offset from the beginning
	@param	size	the size of the data to write
	@param	data	the data to write */
	inline void write(const GLsizeiptr& offset, const GLsizeiptr& size, const void* data) {
		std::memcpy(reinterpret_cast<unsigned char*>(m_bufferPtr[m_writeIndex]) + offset, data, size);
	}
	/** Wait for the current fence to pass, for the current frame index. */
	inline void beginWriting() {
		// Ensure reading has finished before writing
		WaitForFence(m_readFence[m_writeIndex]);
	}
	/** Create a fence for the current point in time. */
	inline void endWriting() {
		if (!m_writeFence[m_writeIndex])
			m_writeFence[m_writeIndex] = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
		m_writeIndex = (m_writeIndex + 1) % BufferCount;
	}
	/***/
	inline void endReading() {
		if (!m_readFence[m_readIndex])
			m_readFence[m_readIndex] = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
		m_readIndex = (m_readIndex + 1) % BufferCount;
	}
	/** Move GL object from 1 instance to another. */
	inline StaticTripleBuffer& operator=(StaticTripleBuffer&& other) noexcept {
		for (int x = 0; x < BufferCount; ++x) {
			m_bufferID[x] = std::move(other.m_bufferID[x]);
			m_bufferPtr[x] = std::move(other.m_bufferPtr[x]);
			m_writeFence[x] = std::move(other.m_writeFence[x]);
			m_readFence[x] = std::move(other.m_readFence[x]);
			other.m_bufferID[x] = 0;
			other.m_bufferPtr[x] = nullptr;
			other.m_writeFence[x] = nullptr;
			other.m_readFence[x] = nullptr;
		}
		m_mapFlags = (std::move(other.m_mapFlags));
		m_writeIndex = std::move(other.m_writeIndex);
		m_readIndex = std::move(other.m_readIndex);
		m_size = (std::move(other.m_size));
		other.m_mapFlags = 0;
		other.m_writeIndex = 0;
		other.m_readIndex = 0;
		other.m_size = 0;
		return *this;
	}


private:
	// Private Methods
	/** Wait for the fence at the supplied index to pass.
	@param	fence			the fence belonging to a particular internal buffer. */
	static void WaitForFence(GLsync& fence) {
		while (fence) {
			GLbitfield waitFlags = 0;
			if (auto waitReturn = glClientWaitSync(fence, waitFlags, 1);
				waitReturn == GL_SIGNALED || waitReturn == GL_ALREADY_SIGNALED || waitReturn == GL_CONDITION_SATISFIED) {
				glDeleteSync(fence);
				fence = nullptr;
				return;
			}
			waitFlags = GL_SYNC_FLUSH_COMMANDS_BIT;
		}
	}


	// Private Attributes
	GLbitfield m_mapFlags = GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;
	mutable GLsync m_writeFence[BufferCount], m_readFence[BufferCount];
	GLuint m_bufferID[BufferCount];
	void* m_bufferPtr[BufferCount];
	int m_writeIndex = 1, m_readIndex = 0;
	size_t m_size = 0ull;
};

#endif // STATICTRIPLEBUFFER_H