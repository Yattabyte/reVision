#pragma once
#ifndef LIGHTING_BUFFER
#define LIGHTING_BUFFER
#ifdef	ENGINE_EXE_EXPORT
#define DT_ENGINE_API 
#elif	ENGINE_DLL_EXPORT 
#define DT_ENGINE_API __declspec(dllexport)
#else
#define	DT_ENGINE_API __declspec(dllimport)
#endif
#define GLEW_STATIC

#include "GL\glew.h"
#include "glm\glm.hpp"

using namespace glm;
class VisualFX;


/**
 * A specialized framebuffer that accumulates lighting information for a single frame.
 * Supports bloom, accumulates over-brightened lights in a second render-target.
 **/
class DT_ENGINE_API Lighting_Buffer
{
public:
	// (de)Constructors
	/** Destroy the lighting buffer. */
	~Lighting_Buffer();
	/** Destroy the lighting buffer. */
	Lighting_Buffer();


	// Public Methods
	/** Initialize the framebuffer.
	 * @param	size			the size of the framebuffer
	 * @param	visualFX		reference to the post-processing utility class
	 * @param	bloomStrength	intensity / number of passes for the bloom effect
	 * @param	depthStencil	reference to the depthStencil texture from the gBuffer */
	void initialize(const vec2 & size, const GLuint & depthStencil);
	/** Binds and clears out all the render-targets in this framebuffer. */
	void clear();
	/** Binds the framebuffer and its render-targets for writing. */
	void bindForWriting();
	/** Binds the framebuffer and its render-targets for reading. */
	void bindForReading();
	/** Change the size of the framebuffer object. 
	 * @param	size			the new size of the framebuffer */
	void resize(const vec2 & size);
	/** Apply blur filter to bloom attachment, finishing the bloom effect. */
	void end();
	

private:
	// Private Attributes
	GLuint m_fbo, m_texture;
	vec2 m_renderSize;
	GLuint m_depth_stencil; // Donated by the geometry buffer
	bool m_Initialized;
};

#endif // LIGHTING_BUFFER