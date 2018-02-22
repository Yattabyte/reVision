#pragma once
#ifndef HDR_BUFFER
#define HDR_BUFFER
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


/**
 * A specialized framebuffer used for combining previous rendering phases and applying HDR+Bloom.
 **/
class DT_ENGINE_API HDR_Buffer
{
public:
	// (de)Constructors
	/** Destroy the HDRBuffer. */
	~HDR_Buffer();
	/** Construct the HDRBuffer. */
	HDR_Buffer();


	// Public Methods
	/** Initialize the framebuffer.
	 * @param	size	the size of the framebuffer */
	void initialize(const vec2 & size);
	/** Binds and clears out all the render-targets in this framebuffer. */
	void clear();
	/** Binds the framebuffer and its render-targets for writing. */
	void bindForWriting();
	/** Binds the framebuffer and its render-targets for reading. */
	void bindForReading();
	/** Change the size of the framebuffer object.
	 * @param	size	the new size of the framebuffer */
	void resize(const vec2 & size);


private:
	// Private Attributes
	GLuint m_fbo;
	GLuint m_texture;
	bool m_Initialized;
};

#endif // HDR_BUFFER