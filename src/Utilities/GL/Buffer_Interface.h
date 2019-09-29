#pragma once
#ifndef BUFFER_INTERFACE_H
#define BUFFER_INTERFACE_H

#include <glad/glad.h>


/** An interface for opengl buffers. */
class Buffer_Interface {
public:
	// Public (de)Constructors
	/** Destroy this buffer. */
	inline virtual ~Buffer_Interface() = default;
	/** Construct a buffer. */
	inline Buffer_Interface() = default;


	// Public Interface Declarations
	/** Bind this buffer to the target specified.
	@param	target			the target type of this buffer. */
	virtual void bindBuffer(const GLenum& target) const = 0;
	/** Bind this buffer to a particular binding point, such as for shaders to read.
	@param	target			the target type of this buffer.
	@param	index			the binding point index to use. */
	virtual void bindBufferBase(const GLenum& target, const GLuint& index) const = 0;
};

#endif // BUFFER_INTERFACE_H