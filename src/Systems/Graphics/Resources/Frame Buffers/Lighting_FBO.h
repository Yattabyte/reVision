#pragma once
#ifndef LIGHTING_BUFFER_H
#define LIGHTING_BUFFER_H

#include "Utilities\GL\FrameBuffer.h"

using namespace glm;
class Engine;
class VisualFX;


/**
 * A specialized framebuffer that accumulates lighting information for a single frame.
 * Supports bloom, accumulates over-brightened lights in a second render-target.
 **/
class Lighting_FBO : public FrameBuffer
{
public:
	// (de)Constructors
	/** Destroy the lighting buffer. */
	~Lighting_FBO();
	/** Construct the lighting buffer. */
	Lighting_FBO();


	// Public Methods
	/** Initialize the framebuffer.
	 * @param	engine	the engine pointer
	 * @param	depthStencil	reference to the depthStencil texture from the geometryFBO */
	void initialize(Engine * engine, const GLuint & depthStencil);	
	/** Binds the framebuffer and its render-targets for writing. */
	virtual void bindForWriting();
	/** Binds the framebuffer and its render-targets for reading. */
	virtual void bindForReading();
	/** Change the size of the framebuffer object. 
	 * @param	size			the new size of the framebuffer */
	virtual void resize(const ivec2 & size);
	/** Apply blur filter to bloom attachment, finishing the bloom effect. */
	void end();
	

private:
	// Private Attributes
	GLuint  m_texture, m_depth_stencil; // Donated by the geometry buffer
	Engine * m_engine;
};

#endif // LIGHTING_BUFFER_H