#pragma once
#ifndef DYNAMICBUFFER_H
#define DYNAMICBUFFER_H

#include "Utilities/GL/glad/glad.h"
#include <utility>


/** An OpenGL framebuffer encapsulation, which can change in size. */
class DynamicBuffer {
public:
	// Public (de)Constructors
	inline ~DynamicBuffer() {
		if (m_bufferID != 0) {
			glUnmapNamedBuffer(m_bufferID[0]);
			glUnmapNamedBuffer(m_bufferID[1]);
			glUnmapNamedBuffer(m_bufferID[2]);
			glDeleteBuffers(3, m_bufferID);
		}
	}
	/** Default. */
	inline DynamicBuffer(const GLsizeiptr & capacity = 256, const void * data = 0, const GLbitfield & mapFlags = GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT)
		: m_maxCapacity(capacity), m_mapFlags(mapFlags) {
		glCreateBuffers(3, m_bufferID);
		for (int x = 0; x < 3; ++x) {
			glNamedBufferStorage(m_bufferID[x], m_maxCapacity, data, GL_DYNAMIC_STORAGE_BIT | m_mapFlags);
			m_bufferPtr[x] = glMapNamedBufferRange(m_bufferID[x], 0, m_maxCapacity, m_mapFlags);
		}
	}
	/** Move gl object from 1 instance to another. */
	inline DynamicBuffer & operator=(DynamicBuffer && o) noexcept {
		m_bufferID[0] = (std::move(o.m_bufferID[0]));
		m_bufferID[1] = (std::move(o.m_bufferID[1]));
		m_bufferID[2] = (std::move(o.m_bufferID[2]));
		m_bufferPtr[0] = (std::move(o.m_bufferPtr[0]));
		m_bufferPtr[1] = (std::move(o.m_bufferPtr[1]));
		m_bufferPtr[2] = (std::move(o.m_bufferPtr[2]));
		m_maxCapacity = (std::move(o.m_maxCapacity));
		m_mapFlags = (std::move(o.m_mapFlags));
		o.m_bufferID[0] = 0;
		o.m_bufferID[1] = 0;
		o.m_bufferID[2] = 0;
		o.m_bufferPtr[0] = nullptr;
		o.m_bufferPtr[1] = nullptr;
		o.m_bufferPtr[2] = nullptr;
		o.m_maxCapacity = 0;
		o.m_mapFlags = 0;
		return *this;
	}


	// Public Methods
	/** Bind this buffer.
	@param	target		the target type of this buffer.
	@param	frameIndex	which frame of this triple buffer to use (0-2).
	*/
	inline void bindBuffer(const GLenum & target, const size_t & frameIndex) const {
		glBindBuffer(target, m_bufferID[frameIndex]);
	}
	/** Bind this buffer to a particular binding point for shaders.
	@param	target		the target type of this buffer.
	@param	index		the binding point index to use.	
	@param	frameIndex	which frame of this triple buffer to use (0-2). */
	inline void bindBufferBase(const GLenum & target, const GLuint & index, const size_t & frameIndex) const {
		glBindBufferBase(target, index, m_bufferID[frameIndex]);
	}
	/** Cast this buffer's pointer to a type, as to allow modifying its underlying data.
	@param	frameIndex	which frame of this triple buffer to use (0-2).
	@return				the pointer to this data in memory, cast to the type specified.
	@param	<T>			the type to cast this to. */
	template <typename T>
	inline T castPointer(const size_t & frameIndex) {
		return reinterpret_cast<T>(m_bufferPtr[frameIndex]);
	}
	/** Expand this buffer to fit the size provided.
	@param	size		the size to expand up to (if not already larger) */
	inline void setMaxSize(const GLsizeiptr & size) {
		expandToFit(0, size);
	}
	/** Write the supplied data to GPU memory.
	@param	frameIndex	which frame of this triple buffer to use (0-2).
	@param	offset		byte offset from the beginning.
	@param	size		the size of the data to write.
	@param	data		the data to write. */
	inline void write(const size_t & frameIndex, const GLsizeiptr & offset, const GLsizeiptr & size, const void * data) {
		expandToFit(offset, size);

		std::memcpy(reinterpret_cast<unsigned char*>(m_bufferPtr[frameIndex]) + offset, data, size);
	}
	/** Write the supplied data to GPU memory.
	@param	frameIndex	which frame of this triple buffer to use (0-2).
	@param	offset		byte offset from the beginning.
	@param	size		the size of the data to write.
	@param	data		the data to write. */
	inline void write_immediate(const GLsizeiptr & offset, const GLsizeiptr & size, const void * data) {
		expandToFit(offset, size);

		glNamedBufferSubData(m_bufferID[0], offset, size, data);
		glNamedBufferSubData(m_bufferID[1], offset, size, data);
		glNamedBufferSubData(m_bufferID[2], offset, size, data);
	}
	/** Expands this buffer's container if it can't fit the specified range to write into
	@note Technically creates a new a new buffer to replace the old one and copies the old data
	@param	offset	byte offset from the beginning
	@param	size	the size of the data to write */
	inline void expandToFit(const GLsizeiptr & offset, const GLsizeiptr & size) {
		if (offset + size > m_maxCapacity) {
			// Create new buffer large enough to fit old data + new data
			const GLsizeiptr oldSize = m_maxCapacity;
			m_maxCapacity += offset + (size * 2);

			for (int x = 0; x < 3; ++x) {
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
	/** Create a fence for the current point in time.
	@param	frameIndex		which frame of this triple buffer to use (0-2). */
	inline void lockFrame(const size_t & frameIndex) {
		if (m_fence[frameIndex])
			glDeleteSync(m_fence[frameIndex]);
		m_fence[frameIndex] = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
	}
	/** Wait for the current fence to pass, for the current frame index.
	@param	frameIndex		which frame of this triple buffer to use (0-2). */
	inline void waitFrame(const size_t & frameIndex) {
		if (m_fence[frameIndex] != nullptr)
			while (1) {
				GLenum waitReturn = glClientWaitSync(m_fence[frameIndex], GL_SYNC_FLUSH_COMMANDS_BIT, 1);
				if (waitReturn == GL_SIGNALED || waitReturn == GL_ALREADY_SIGNALED || waitReturn == GL_CONDITION_SATISFIED) {
					glDeleteSync(m_fence[frameIndex]);
					m_fence[frameIndex] = nullptr;
					return;
				}
			}
	}


private:
	// Private Attributes
	GLuint m_bufferID[3] = { 0,0,0 };
	void * m_bufferPtr[3] = { nullptr, nullptr, nullptr };
	GLsync m_fence[3] = { nullptr, nullptr, nullptr };
	GLsizeiptr m_maxCapacity = 256;
	GLbitfield m_mapFlags = GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;
};

#endif // DYNAMICBUFFER_H