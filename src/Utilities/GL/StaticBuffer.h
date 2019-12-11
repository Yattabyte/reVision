#pragma once
#ifndef STATICBUFFER_H
#define STATICBUFFER_H

#include "Utilities/GL/Buffer_Interface.h"


/** Encapsulates an OpenGL buffer that is fixed in size. */
class StaticBuffer final : public Buffer_Interface {
public:
	// Public (De)Constructors
	/** Destroy this buffer. */
	~StaticBuffer() noexcept;
	/** Default Constructor. */
	/** Construct a static buffer. */
	inline StaticBuffer() noexcept = default;
	/** Explicit Instantiation. */
	explicit StaticBuffer(const GLsizeiptr& size, const void* data = 0, const GLbitfield& storageFlags = GL_DYNAMIC_STORAGE_BIT) noexcept;
	/** Construct and copy another static buffer.
	@param	other	another buffer to copy from. */
	StaticBuffer(const StaticBuffer& other) noexcept;
	/** Construct and move from another static buffer.
	@param	other	another buffer to move from. */
	StaticBuffer(StaticBuffer&& other) noexcept;
	/** Copy OpenGL object from 1 instance to another.
	@param	other	another buffer to copy from.
	@return			reference to this. */
	StaticBuffer& operator=(const StaticBuffer& other) noexcept;
	/** Move OpenGL object from 1 instance to another.
	@param	other	another buffer to move from.
	@return			reference to this. */
	StaticBuffer& operator=(StaticBuffer&& other) noexcept;


	// Public Interface Implementations
	void bindBuffer(const GLenum& target) const noexcept final;
	void bindBufferBase(const GLenum& target, const GLuint& index) const noexcept final;


	// Public Methods
	/** Write the supplied data to GPU memory.
	@param	offset	byte offset from the beginning.
	@param	size	the size of the data to write.
	@param	data	the data to write. */
	void write(const GLsizeiptr& offset, const GLsizeiptr& size, const void* data) noexcept;


private:
	// Private Attributes
	GLuint m_bufferID = 0;
	size_t m_size = 0ull;
	GLbitfield m_storageFlags = GL_DYNAMIC_STORAGE_BIT;
};

#endif // STATICBUFFER_H