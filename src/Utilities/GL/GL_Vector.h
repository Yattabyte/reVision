#pragma once
#ifndef GL_VECTOR_H
#define GL_VECTOR_H

#include "Utilities/GL/Buffer_Interface.h"
#include <memory>
#include <vector>


/** A multi-buffered STL vector like class, storing its data on the GPU using persistently mapped coherent buffers.
@param	<T>				the type of element to construct an array of. */
template <typename T, int BufferCount = 3>
class GL_Vector final : public Buffer_Interface {
public:
	// Public (De)Constructors
	/** Destroy this GL Vector. */
	inline ~GL_Vector() noexcept {
		// Safely destroy each buffer this class owns
		for (int x = 0; x < BufferCount; ++x) {
			WaitForFence(m_writeFence[x]);
			WaitForFence(m_readFence[x]);
			if (m_bufferID[x]) {
				glUnmapNamedBuffer(m_bufferID[x]);
				glDeleteBuffers(1, &m_bufferID[x]);
			}
		}
	}
	/** Construct a GL Vector.
	@param	capacity		the starting capacity (1 or more). */
	inline GL_Vector(const size_t& capacity = 1) noexcept : m_capacity(std::max(1ull, capacity)) {
		// Zero-initialize our starting variables
		for (int x = 0; x < BufferCount; ++x) {
			m_bufferID[x] = 0;
			m_bufferPtr[x] = nullptr;
			m_writeFence[x] = nullptr;
			m_readFence[x] = nullptr;
		}

		// Create 'BufferCount' number of buffers & map them
		const auto bufferSize = sizeof(T) * m_capacity;
		glCreateBuffers(BufferCount, m_bufferID);
		for (int x = 0; x < BufferCount; ++x) {
			glNamedBufferStorage(m_bufferID[x], bufferSize, 0, GL_DYNAMIC_STORAGE_BIT | BufferFlags);
			m_bufferPtr[x] = static_cast<T*>(glMapNamedBufferRange(m_bufferID[x], 0, bufferSize, BufferFlags));
		}
	}
	/** Assignment constructor.
	@param	other			another buffer to move the data from, to here. */
	inline GL_Vector(GL_Vector&& other) noexcept {
		(*this) = std::move(other);
	}


	// Public Interface Implementations
	inline virtual void bindBuffer(const GLenum& target) const noexcept override final {
		// Ensure writing has finished before reading
		//WaitForFence(m_writeFence[m_index]);
		glBindBuffer(target, m_bufferID[m_index]);
	}
	inline virtual void bindBufferBase(const GLenum& target, const GLuint& index) const noexcept override final {
		// Ensure writing has finished before reading
		//WaitForFence(m_writeFence[m_index]);
		glBindBufferBase(target, index, m_bufferID[m_index]);
	}


	// Public Methods
	/** Prepare this buffer for writing, waiting on any unfinished reads. */
	inline void beginWriting() const noexcept {
		// Ensure all reads and writes at this index have finished.
		WaitForFence(m_writeFence[m_index]);
		WaitForFence(m_readFence[m_index]);
	}
	/** Signal that this multi-buffer is finished being written to. */
	inline void endWriting() const noexcept {
		if (!m_writeFence[m_index])
			m_writeFence[m_index] = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
	}
	/** Signal that this multi-buffer is finished being read from. */
	inline void endReading() noexcept {
		if (!m_readFence[m_index])
			m_readFence[m_index] = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
		m_index = (m_index + 1) % BufferCount;
	}
	/** Resizes the internal capacity of this vector.
	@note					Does nothing if the capacity is the same
	@note					Currently, only grows, never shrinks
	@note					May stall waiting for old buffers to finish, invalidate them.
	@param	newCapacity		the new desired capacity. */
	inline void resize(const size_t& newCapacity) noexcept {
		// See if we must expand this container
		if (newCapacity > m_capacity) {
			// Calculate old and new byte sizes
			const auto oldByteSize = sizeof(T) * m_capacity;
			const auto newByteSize = sizeof(T) * newCapacity;
			m_capacity = newCapacity;

			// Wait for and transfer data from old buffers into new buffers of the new size
			for (int x = 0; x < BufferCount; ++x) {
				WaitForFence(m_writeFence[x]);
				WaitForFence(m_readFence[x]);

				// Create new buffer
				GLuint newBuffer = 0;
				glCreateBuffers(1, &newBuffer);
				glNamedBufferStorage(newBuffer, newByteSize, 0, GL_DYNAMIC_STORAGE_BIT | BufferFlags);

				// Copy old buffer
				if (oldByteSize)
					glCopyNamedBufferSubData(m_bufferID[x], newBuffer, 0, 0, oldByteSize);

				// Delete old buffer
				glUnmapNamedBuffer(m_bufferID[x]);
				glDeleteBuffers(1, &m_bufferID[x]);

				// Migrate new buffer
				m_bufferID[x] = newBuffer;
				m_bufferPtr[x] = (T*)(glMapNamedBufferRange(m_bufferID[x], 0, newByteSize, BufferFlags));
			}
		}
	}
	/** Retrieve the length of this vector (the number of elements in it).
	@return					the number of elements in this array. */
	inline size_t getLength() const noexcept {
		return m_capacity;
	}
	/** Assignment operator, for moving another buffer into this one.
	@param	other			another buffer to move the data from, to here. */
	inline GL_Vector& operator=(GL_Vector&& other) noexcept {
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

		m_capacity = std::move(other.m_capacity);
		m_index = std::move(other.m_index);
		other.m_capacity = 0;
		other.m_index = 0;
		return *this;
	}
	/** Index operator, retrieve a reference to the element at the index specified.
	@param	index			an index to the element desired.
	@return					reference to the element desired. */
	inline T& operator [] (const size_t& index) noexcept {
		return m_bufferPtr[m_index][index];
	}


private:
	// Private Methods
	/** Wait for the fence at the supplied index to pass.
	@param	fence			the fence belonging to a particular internal buffer. */
	static void WaitForFence(GLsync& fence) noexcept {
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
	constexpr const static GLbitfield BufferFlags = GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;
	mutable GLsync m_writeFence[BufferCount]{}, m_readFence[BufferCount]{};
	GLuint m_bufferID[BufferCount]{};
	T* m_bufferPtr[BufferCount]{};
	int m_index = 0;
	size_t m_capacity = 0;
};

#endif // GL_VECTOR_H
