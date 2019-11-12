#pragma once
#ifndef DYNAMICBUFFER_H
#define DYNAMICBUFFER_H

#include "Utilities/GL/Buffer_Interface.h"
#include <utility>


/** An OpenGL framebuffer encapsulation, which can change in size. */
template <int BufferCount = 3>
class DynamicBuffer final : public Buffer_Interface {
public:
	// Public (De)Constructors
	/** Wait for all fences to complete, then destroy this buffer. */
	inline ~DynamicBuffer() {
		for (int x = 0; x < BufferCount; ++x) {
			WaitForFence(m_writeFence[x]);
			WaitForFence(m_readFence[x]);
			if (m_bufferID[x]) {
				glUnmapNamedBuffer(m_bufferID[x]);
				glDeleteBuffers(1, &m_bufferID[x]);
			}
		}
	}
	/** Construct a new Dynamic buffer.
	@param	capacity	the starting capacity of this buffer.
	@param	data		optional data buffer, must be at least as large.
	@param	mapFlags	bit-field flags. */
	inline DynamicBuffer(const GLsizeiptr& capacity = 256, const void* data = 0, const GLbitfield& mapFlags = GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT)
		: m_mapFlags(mapFlags), m_maxCapacity(capacity) {
		// Zero-initialize our starting variables
		for (int x = 0; x < BufferCount; ++x) {
			m_bufferID[x] = 0;
			m_bufferPtr[x] = nullptr;
			m_writeFence[x] = nullptr;
			m_readFence[x] = nullptr;
		}

		glCreateBuffers(BufferCount, m_bufferID);
		for (int x = 0; x < BufferCount; ++x) {
			glNamedBufferStorage(m_bufferID[x], m_maxCapacity, data, GL_DYNAMIC_STORAGE_BIT | m_mapFlags);
			m_bufferPtr[x] = glMapNamedBufferRange(m_bufferID[x], 0, m_maxCapacity, m_mapFlags);
		}
	}
	/** Construct a new Dynamic buffer, from another buffer. */
	inline DynamicBuffer(const DynamicBuffer& other) : DynamicBuffer(other.m_maxCapacity, 0, other.m_mapFlags) {
		for (int x = 0; x < BufferCount; ++x)
			glCopyNamedBufferSubData(other.m_bufferID[x], m_bufferID[x], 0, 0, m_maxCapacity);
	}
	/** Assignment constructor.
	@param	other			another buffer to move the data from, to here. */
	inline DynamicBuffer(DynamicBuffer&& other) noexcept {
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
	/** Expand this buffer to fit the size provided.
	@param	size		the size to expand up to (if not already larger) */
	inline void setMaxSize(const GLsizeiptr& size) {
		expandToFit(0, size);
	}
	/** Write the supplied data to GPU memory.
	@param	offset		byte offset from the beginning.
	@param	size		the size of the data to write.
	@param	data		the data to write. */
	inline void write(const GLsizeiptr& offset, const GLsizeiptr& size, const void* data) {
		expandToFit(offset, size);
		std::memcpy(reinterpret_cast<unsigned char*>(m_bufferPtr[m_index]) + offset, data, size);
	}
	/** Write the supplied data to GPU memory.
	@param	offset		byte offset from the beginning.
	@param	size		the size of the data to write.
	@param	data		the data to write. */
	inline void write_immediate(const GLsizeiptr& offset, const GLsizeiptr& size, const void* data) {
		expandToFit(offset, size);

		for each (const auto & buffer in m_bufferID)
			glNamedBufferSubData(buffer, offset, size, data);
	}
	/** Expands this buffer's container if it can't fit the specified range to write into
	@note Technically creates a new a new buffer to replace the old one and copies the old data
	@param	offset	byte offset from the beginning
	@param	size	the size of the data to write */
	inline void expandToFit(const GLsizeiptr& offset, const GLsizeiptr& size) {
		if (offset + size > m_maxCapacity) {
			// Create new buffer large enough to fit old data + new data
			const GLsizeiptr oldSize = m_maxCapacity;
			m_maxCapacity += offset + (size * 2);

			// Wait for and transfer data from old buffers into new buffers of the new size
			for (int x = 0; x < BufferCount; ++x) {
				WaitForFence(m_writeFence[x]);
				WaitForFence(m_readFence[x]);

				// Create new buffer
				GLuint newBuffer = 0;
				glCreateBuffers(1, &newBuffer);
				glNamedBufferStorage(newBuffer, m_maxCapacity, 0, GL_DYNAMIC_STORAGE_BIT | m_mapFlags);

				// Copy old buffer
				if (oldSize)
					glCopyNamedBufferSubData(m_bufferID[x], newBuffer, 0, 0, oldSize);

				// Delete old buffer
				glUnmapNamedBuffer(m_bufferID[x]);
				glDeleteBuffers(1, &m_bufferID[x]);

				// Migrate new buffer
				m_bufferID[x] = newBuffer;
				m_bufferPtr[x] = glMapNamedBufferRange(m_bufferID[x], 0, m_maxCapacity, m_mapFlags);
			}
		}
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
	/** Signal that this multi-buffer is finished being read from. */
	inline void endReading() {
		if (!m_readFence[m_index])
			m_readFence[m_index] = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
		m_index = (m_index + 1) % BufferCount;
	}
	/** Assignment operator, for moving another buffer into this one.
	@param	other			another buffer to move the data from, to here. */
	inline DynamicBuffer& operator=(DynamicBuffer&& other) noexcept {
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
		m_maxCapacity = (std::move(other.m_maxCapacity));
		m_index = std::move(other.m_index);
		other.m_mapFlags = 0;
		other.m_maxCapacity = 0;
		other.m_index = 0;
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
	mutable GLsync m_writeFence[BufferCount]{}, m_readFence[BufferCount]{};
	GLuint m_bufferID[BufferCount]{};
	void* m_bufferPtr[BufferCount]{};
	int m_index = 0;
	GLsizeiptr m_maxCapacity = 256;
};

#endif // DYNAMICBUFFER_H