#pragma once
#ifndef GL_ARRAYBUFFER_H
#define GL_ARRAYBUFFER_H

#include "Utilities/GL/Buffer_Interface.h"
#include <memory>
#include <vector>


/** A data buffer that resides locally as well as on the GPU. 
Forms an expandable array of elements type T.
@param	<T>		the type of element to construct an array of. */
template <typename T>
class GL_ArrayBuffer : public Buffer_Interface {
public:
	// Public (de)Constructors
	/** Destroy this buffer. */
	inline ~GL_ArrayBuffer() {
		for (int x = 0; x < 3; ++x) {
			if (m_fence[x] != nullptr)
				glDeleteSync(m_fence[x]);
			if (m_bufferID[x]) {
				glUnmapNamedBuffer(m_bufferID[x]);
				glDeleteBuffers(1, &m_bufferID[x]);
			}
		}
	}
	/** Construct a buffer.
	@param	capacity		the starting capacity (1 or more). */
	inline GL_ArrayBuffer(const size_t & capacity = 1) : m_capacity(std::max<size_t>(1ull, capacity)) {
		// WRITE ONLY, DON'T READ FROM IT (on our side, not GPU side)
		constexpr GLbitfield flags = GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;
		const auto bufferSize = sizeof(T) * m_capacity;
		glCreateBuffers(3, m_bufferID);
		for (int x = 0; x < 3; ++x) {
			glNamedBufferStorage(m_bufferID[x], bufferSize, 0, GL_DYNAMIC_STORAGE_BIT | flags);
			m_bufferPtr[x] = (T*)glMapNamedBufferRange(m_bufferID[x], 0, bufferSize, flags);
		}
	}
	/** Assignment constructor, for moving 1 buffer to another.
	@param	o				the other buffer to move to here (invalidating it). */
	inline GL_ArrayBuffer(GL_ArrayBuffer && o) {
		m_bufferID[0] = (std::move(o.m_bufferID[0]));
		m_bufferID[1] = (std::move(o.m_bufferID[1]));
		m_bufferID[2] = (std::move(o.m_bufferID[2]));
		m_bufferPtr[0] = (std::move(o.m_bufferPtr[0]));
		m_bufferPtr[1] = (std::move(o.m_bufferPtr[1]));
		m_bufferPtr[2] = (std::move(o.m_bufferPtr[2]));
		m_capacity = std::move(o.m_capacity);

		o.m_bufferID[0] = 0;
		o.m_bufferID[1] = 0;
		o.m_bufferID[2] = 0;
		o.m_bufferPtr[0] = nullptr;
		o.m_bufferPtr[1] = nullptr;
		o.m_bufferPtr[2] = nullptr;
		o.m_capacity = 0;
	}
	/** Assignment operator, for moving 1 buffer to another. 
	@param	o				the other buffer to move to here (invalidating it). */
	inline GL_ArrayBuffer & operator=(GL_ArrayBuffer && o) noexcept {
		m_bufferID[0] = (std::move(o.m_bufferID[0]));
		m_bufferID[1] = (std::move(o.m_bufferID[1]));
		m_bufferID[2] = (std::move(o.m_bufferID[2]));
		m_bufferPtr[0] = (std::move(o.m_bufferPtr[0]));
		m_bufferPtr[1] = (std::move(o.m_bufferPtr[1]));
		m_bufferPtr[2] = (std::move(o.m_bufferPtr[2]));
		m_capacity = std::move(o.m_capacity);

		o.m_bufferID[0] = 0;
		o.m_bufferID[1] = 0;
		o.m_bufferID[2] = 0;
		o.m_bufferPtr[0] = nullptr;
		o.m_bufferPtr[1] = nullptr;
		o.m_bufferPtr[2] = nullptr;
		o.m_capacity = 0;
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
	inline void resize(const size_t & size) {
		// See if we must expand this container
		if (size > m_capacity) {
			const auto oldSize = sizeof(T) * m_capacity;

			m_capacity = size;
			const auto newSize = sizeof(T) * m_capacity;

			for (int x = 0; x < 3; ++x) {
				// Wait for this buffer in particular
				if (m_fence[x] != nullptr)
					while (1) {
						GLenum waitReturn = glClientWaitSync(m_fence[x], GL_SYNC_FLUSH_COMMANDS_BIT, 1);
						if (waitReturn == GL_SIGNALED || waitReturn == GL_ALREADY_SIGNALED || waitReturn == GL_CONDITION_SATISFIED) {
							glDeleteSync(m_fence[x]);
							m_fence[x] = nullptr;
							break;
						}
					}


				// Create new buffer
				constexpr GLbitfield flags = GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;
				GLuint newBuffer = 0;
				glCreateBuffers(1, &newBuffer);
				glNamedBufferStorage(newBuffer, newSize, 0, GL_DYNAMIC_STORAGE_BIT | flags);

				// Copy old buffer
				if (oldSize)
					glCopyNamedBufferSubData(m_bufferID[x], newBuffer, 0, 0, oldSize);

				// Delete old buffer
				glUnmapNamedBuffer(m_bufferID[x]);
				glDeleteBuffers(1, &m_bufferID[x]);

				// Migrate new buffer
				m_bufferID[x] = newBuffer;
				m_bufferPtr[x] = (T*)glMapNamedBufferRange(m_bufferID[x], 0, newSize, flags);
			}
		}
	}
	/** Retrieve a reference to the element contained at the index specified.
	@param	index			index to the element desired.
	@return					reference to the element desired. */
	inline T& operator [] (const size_t & index) {
		return m_bufferPtr[m_writeIndex][index];
	}
	/** Retrieve the length of this array (the number of elements in it).
	@return					the number of elemetns in this array. */
	inline size_t getLength() const {
		return m_capacity;
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
	T  * m_bufferPtr[3] = { nullptr, nullptr, nullptr };
	GLsync m_fence[3] = { nullptr, nullptr, nullptr };
	size_t m_capacity = 0;
};

#endif // GL_ARRAYBUFFER_H