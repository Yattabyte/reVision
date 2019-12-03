#pragma once
#ifndef STATICBUFFER_H
#define STATICBUFFER_H

#include "Utilities/GL/Buffer_Interface.h"
#include <utility>


/** Encapsulates an OpenGL buffer that is fixed in size. */
class StaticBuffer final : public Buffer_Interface {
public:
	// Public (De)Constructors
	/** Destroy this buffer. */
	inline ~StaticBuffer() noexcept {
		if (m_bufferID != 0)
			glDeleteBuffers(1, &m_bufferID);
	}
	/** Default Constructor. */
	/** Construct a static buffer. */
	inline StaticBuffer() = default;
	/** Explicit Instantiation. */
	inline explicit StaticBuffer(const GLsizeiptr& size, const void* data = 0, const GLbitfield& storageFlags = GL_DYNAMIC_STORAGE_BIT) noexcept
		: m_size(size), m_storageFlags(storageFlags) {
		glCreateBuffers(1, &m_bufferID);
		glNamedBufferStorage(m_bufferID, size, data, storageFlags);
	}
	/** Construct and copy another static buffer.
	@param	other	another buffer to copy from. */
	inline StaticBuffer(const StaticBuffer& other) noexcept
		: StaticBuffer(other.m_size, 0, other.m_storageFlags) {
		glCopyNamedBufferSubData(other.m_bufferID, m_bufferID, 0, 0, other.m_size);
	}
	/** Construct and move from another static buffer.
	@param	other	another buffer to move from. */
	inline StaticBuffer(StaticBuffer&& other) noexcept {
		(*this) = std::move(other);
	}
	/** Move OpenGL object from 1 instance to another.
	@param	other	another buffer to move from.
	@return			reference to this. */
	inline StaticBuffer& operator=(StaticBuffer&& other) noexcept {
		if (this != &other) {
			m_bufferID = other.m_bufferID;
			other.m_bufferID = 0;
		}
		return *this;
	}


	// Public Interface Implementations
	inline virtual void bindBuffer(const GLenum& target) const noexcept override final {
		glBindBuffer(target, m_bufferID);
	}
	inline virtual void bindBufferBase(const GLenum& target, const GLuint& index) const noexcept override final {
		glBindBufferBase(target, index, m_bufferID);
	}


	// Public Methods
	/** Write the supplied data to GPU memory.
	@param	offset	byte offset from the beginning.
	@param	size	the size of the data to write.
	@param	data	the data to write. */
	inline void write(const GLsizeiptr& offset, const GLsizeiptr& size, const void* data) noexcept {
		glNamedBufferSubData(m_bufferID, offset, size, data);
	}


private:
	// Private Attributes
	GLuint m_bufferID = 0;
	size_t m_size = 0ull;
	GLbitfield m_storageFlags = GL_DYNAMIC_STORAGE_BIT;
};

#endif // STATICBUFFER_H
