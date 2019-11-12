#pragma once
#ifndef STATICMULTIBUFFER_H
#define STATICMULTIBUFFER_H

#include "Utilities/GL/Buffer_Interface.h"
#include <utility>


/** Encapsulates an OpenGL buffer that is fixed in size. */
template <int BufferCount = 3>
class StaticMultiBuffer final : public Buffer_Interface {
public:
	// Public (de)Constructors
	/** Wait on this buffers fences, then destroy it. */
	inline ~StaticMultiBuffer() {
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
	inline StaticMultiBuffer() {
		// Zero-initialize our starting variables
		for (int x = 0; x < BufferCount; ++x) {
			m_bufferID[x] = 0;
			m_bufferPtr[x] = nullptr;
			m_writeFence[x] = nullptr;
			m_readFence[x] = nullptr;
		}
	}
	/** Construct a new Static Multi-Buffer.
	@param	size			the starting size of this buffer.
	@param	data			optional data buffer, must be at least as large.
	@param	storageFlags	optional bit-field flags. */
	inline explicit StaticMultiBuffer(const GLsizeiptr& size, const void* data = 0, const GLbitfield& storageFlags = GL_DYNAMIC_STORAGE_BIT)
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
	/** Construct a new Static Multi-Buffer, from another buffer. */
	inline StaticMultiBuffer(const StaticMultiBuffer& other)
		: StaticMultiBuffer(other.m_size, 0) {
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
	inline StaticMultiBuffer(StaticMultiBuffer&& other) noexcept {
		(*this) = std::move(other);
	}


	// Public Interface Implementations
	inline virtual void bindBuffer(const GLenum& target) const override final {
		// Ensure writing has finished before reading
		//WaitForFence(m_writeFence[m_index]);
		glBindBuffer(target, m_bufferID[m_index]);
	}
	inline virtual void bindBufferBase(const GLenum& target, const GLuint& index) const override final {
		// Ensure writing has finished before reading
		//WaitForFence(m_writeFence[m_index]);
		glBindBufferBase(target, index, m_bufferID[m_index]);
	}


	// Public Methods
	/** Write the supplied data to GPU memory
	@param	offset			byte offset from the beginning
	@param	size			the size of the data to write
	@param	data			the data to write */
	inline void write(const GLsizeiptr& offset, const GLsizeiptr& size, const void* data) {
		std::memcpy(reinterpret_cast<unsigned char*>(m_bufferPtr[m_index]) + offset, data, size);
	}
	/** Prepare this buffer for writing, waiting on any unfinished reads. */
	inline void beginWriting() const {
		// Ensure all reads and writes at this index have finished.
		WaitForFence(m_writeFence[m_index]);
		WaitForFence(m_readFence[m_index]);
	}
	/** Signal that this multi-buffer is finished being written to. */
	inline void endWriting() const {
		if (!m_writeFence[m_index])
			m_writeFence[m_index] = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
	}
	/** Signal that this multi-buffer has finished being read from. */
	inline void endReading() {
		if (!m_readFence[m_index])
			m_readFence[m_index] = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
		m_index = (m_index + 1) % BufferCount;
	}
	/** Move GL object from 1 instance to another. */
	inline StaticMultiBuffer& operator=(StaticMultiBuffer&& other) noexcept {
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
		m_index = std::move(other.m_index);
		m_size = (std::move(other.m_size));
		other.m_mapFlags = 0;
		other.m_index = 0;
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
	int m_index = 0;
	size_t m_size = 0ull;
};

#endif // STATICMULTIBUFFER_H