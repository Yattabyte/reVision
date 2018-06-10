#pragma once
#ifndef REFLECTION_BUFFER_H
#define REFLECTION_BUFFER_H
#define GLEW_STATIC

#include "Utilities\GL\FrameBuffer.h"

class EnginePackage;


/**
 * A specialized framebuffer that accumulates lighting information for a single frame.
 * Supports bloom, accumulates over-brightened lights in a second render-target.
 **/
class Reflection_FBO : public FrameBuffer
{
public:
	// (de)Constructors
	/** Destroy the reflection buffer. */
	~Reflection_FBO();
	/** Construct the reflection buffer. */
	Reflection_FBO();


	// Public Interface Implementations
	/** Initialize the framebuffer. 
	 * @param	enginePackage	the engine package */
	void initialize(EnginePackage * enginePackage, const GLuint & depthStencil);
	/** Binds the framebuffer and its render-targets for reading. */
	void bindForReading(const unsigned int & texture_unit);
	/** Binds the framebuffer and its render-targets for writing. */
	virtual void bindForWriting();
	/** Change the size of the framebuffer object.
	* @param	size	the new size of the framebuffer */
	virtual void resize(const vec2 & size);
	/** Return the depth-stencil buffer. */
	void end();



private:
	// Private Attributes
	GLuint m_texture, m_depth_stencil; // Donated by the geometry buffer
	EnginePackage * m_enginePackage;
};

#endif // REFLECTION_BUFFER_H