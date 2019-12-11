#pragma once
#ifndef BUFFER_INTERFACE_H
#define BUFFER_INTERFACE_H

#include <glad/glad.h>


/** An interface for OpenGL buffers. */
class Buffer_Interface {
public:
	// Public (De)Constructors
	/** Destroy this buffer. */
	inline virtual ~Buffer_Interface() noexcept = default;
	/** Construct a buffer. */
	inline Buffer_Interface() noexcept = default;
	/** Move a buffer. */
	inline Buffer_Interface(Buffer_Interface&&) noexcept = default;
	/** Copy a buffer. */
	inline Buffer_Interface(const Buffer_Interface&) noexcept = default;


	// Public Interface Declarations
	/** Bind this buffer to the target specified.
	@param	target			the target type of this buffer. */
	virtual void bindBuffer(const GLenum& target) const noexcept = 0;
	/** Bind this buffer to a particular binding point, such as for shaders to read.
	@param	target			the target type of this buffer.
	@param	index			the binding point index to use. */
	virtual void bindBufferBase(const GLenum& target, const GLuint& index) const noexcept = 0;
};

#endif // BUFFER_INTERFACE_H