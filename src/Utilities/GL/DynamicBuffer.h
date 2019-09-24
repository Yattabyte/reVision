#pragma once
#ifndef DYNAMICBUFFER_H
#define DYNAMICBUFFER_H

#include "Utilities/GL/Buffer_Interface.h"
#include <utility>


/** An OpenGL framebuffer encapsulation, which can change in size. */
class DynamicBuffer final : public Buffer_Interface {
public:
	// Public (de)Constructors
	inline ~DynamicBuffer() {
		for (int x = 0; x < 3; ++x) {
			if (m_fence[x] != nullptr)
				glDeleteSync(m_fence[x]);
			if (m_bufferID[x]) {
				glUnmapNamedBuffer(m_bufferID[x]);
				glDeleteBuffers(1, &m_bufferID[x]);
			}
		}
	}
	/** Default. */
	inline DynamicBuffer(const GLsizeiptr& capacity = 256, const void* data = 0, const GLbitfield & mapFlags = GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT)
		: m_maxCapacity(capacity), m_mapFlags(mapFlags) {
		glCreateBuffers(3, m_bufferID);
		for (int x = 0; x < 3; ++x) {
			glNamedBufferStorage(m_bufferID[x], m_maxCapacity, data, GL_DYNAMIC_STORAGE_BIT | m_mapFlags);
			m_bufferPtr[x] = glMapNamedBufferRange(m_bufferID[x], 0, m_maxCapacity, m_mapFlags);
		}
	}
	inline DynamicBuffer(const DynamicBuffer& other)
		: DynamicBuffer(other.m_maxCapacity, 0, other.m_mapFlags) {
		for (int x = 0; x < 3; ++x)
			glCopyNamedBufferSubData(other.m_bufferID[x], m_bufferID[x], 0, 0, m_maxCapacity);
	}
	/** Move gl object from 1 instance to another. */
	inline DynamicBuffer& operator=(DynamicBuffer&& o) noexcept {
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


	// Public Inteface Implementations
	inline virtual void bindBuffer(const GLenum& target) const override final {
		glBindBuffer(target, m_bufferID[m_writeIndex]);
	}
	inline virtual void bindBufferBase(const GLenum& target, const GLuint& index) const override final {
		glBindBufferBase(target, index, m_bufferID[m_writeIndex]);
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
		std::memcpy(reinterpret_cast<unsigned char*>(m_bufferPtr[m_writeIndex]) + offset, data, size);
	}
	/** Write the supplied data to GPU memory.
	@param	offset		byte offset from the beginning.
	@param	size		the size of the data to write.
	@param	data		the data to write. */
	inline void write_immediate(const GLsizeiptr& offset, const GLsizeiptr& size, const void* data) {
		expandToFit(offset, size);

		glNamedBufferSubData(m_bufferID[0], offset, size, data);
		glNamedBufferSubData(m_bufferID[1], offset, size, data);
		glNamedBufferSubData(m_bufferID[2], offset, size, data);
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
	void* m_bufferPtr[3] = { nullptr, nullptr, nullptr };
	GLsync m_fence[3] = { nullptr, nullptr, nullptr };
	GLsizeiptr m_maxCapacity = 256;
	GLbitfield m_mapFlags = GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;
};

#endif // DYNAMICBUFFER_H