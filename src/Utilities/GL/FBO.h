#pragma once
#ifndef FBO_H
#define FBO_H

#include "Utilities\GL\StaticBuffer.h"
#include "Assets\Asset_Shader.h"
#include "Assets\Asset_Primitive.h"


/** An OpenGL FBO usage interface. */
struct FBO_Base {
	// Public Interface
	/** Binds and clears out all the render-targets in this framebuffer. */
	virtual void clear() = 0;
	/** Binds the framebuffer and its render-targets for writing. */
	virtual void bindForWriting() = 0;
	/** Binds the framebuffer and its render-targets for reading. */
	virtual void bindForReading(const GLuint & binding = 0) = 0;
	/** Change the size of the framebuffer object.
	 * @param	sizes		the new sizes of the framebuffer */
	virtual void resize(const GLuint & width = 1, const GLuint & height = 1, const GLuint & depth = 1) = 0;
	/** Attach the given texture to the desired attachment point. 
	 * @param	textureObj	the texture to attach 
	 * @param	attachPoint	the attachment point
	 * @param	level		the level to attach to */
	virtual void attachTexture(const GLuint & textureObj, const GLenum & attachPoint, const GLuint & level = 0) = 0;
};

#endif // FBO_H