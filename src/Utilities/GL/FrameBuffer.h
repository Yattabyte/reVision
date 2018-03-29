#pragma once
#ifndef FRAME_BUFFER
#define FRAME_BUFFER
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
#include "Managers\Message_Manager.h"

using namespace glm;


/**
 * An encapsulation of an OpenGL framebuffer.
 * Requires the implementer to manage the size of the frame buffer, and also add render targets.
 **/
class DT_ENGINE_API FrameBuffer
{
public:
	// (de)Constructors
	/** Destroy the lighting buffer. */
	virtual ~FrameBuffer() {
		if (m_Initialized && m_fbo != 0)
			glDeleteFramebuffers(1, &m_fbo);

	}
	/** Construct the lighting buffer. */
	FrameBuffer() {
		m_Initialized = false;
		m_fbo = 0;
		m_renderSize = ivec2(1);
	}


	// Public Methods
	/** Initialize the framebuffer. */
	virtual void initialize() {
		if (!m_Initialized) {
			glCreateFramebuffers(1, &m_fbo);
			m_Initialized = true;
		}
	}
	/** Binds and clears out all the render-targets in this framebuffer. */
	virtual void clear() {
		GLfloat clearColor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
		glClearNamedFramebufferfv(m_fbo, GL_COLOR, 0, clearColor);
	}
	/** Binds the framebuffer and its render-targets for writing. */
	virtual void bindForWriting() {
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo);
	}
	/** Binds the framebuffer and its render-targets for reading. */
	virtual void bindForReading() {
		return;
	}  
	/** Change the size of the framebuffer object. 
	 * @param	size	the new size of the framebuffer */
	virtual void resize(const ivec2 & size) {
		m_renderSize = size;
	}
	

protected:
	// Protected Methods
	/* Checks if this framebuffer is complete.
	 * @return true if complete, false otherwise */
	bool validate() {
		const GLenum Status = glCheckNamedFramebufferStatus(m_fbo, GL_FRAMEBUFFER);
		if (Status != GL_FRAMEBUFFER_COMPLETE && Status != GL_NO_ERROR) {
			MSG_Manager::Error(MSG_Manager::FBO_INCOMPLETE, "" , std::string(reinterpret_cast<char const *>(glewGetErrorString(Status))));		
			return false;
		}
		return true;
	}


	// Protected Attributes
	bool m_Initialized;
	GLuint m_fbo;
	ivec2 m_renderSize;
};

#endif // FRAME_BUFFER