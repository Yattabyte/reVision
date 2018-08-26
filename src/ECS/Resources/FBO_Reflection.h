#pragma once
#ifndef FBO_REFLECTION_H
#define FBO_REFLECTION_H 

#include "Utilities\GL\FBO.h"
#include "GLFW\glfw3.h"


/** A framebuffer, formatted for rendering reflections. */
struct FBO_Reflection : FBO_Base {
	GLuint m_fboID, m_textureID;
	glm::ivec2 m_size;
	~FBO_Reflection() {
		glDeleteFramebuffers(1, &m_fboID);
		glDeleteTextures(1, &m_textureID);
	}
	FBO_Reflection() {
		m_fboID = 0;
		m_textureID = 0;
		glCreateFramebuffers(1, &m_fboID);
		glCreateTextures(GL_TEXTURE_2D, 1, &m_textureID);
		resize();
		glTextureParameteri(m_textureID, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTextureParameteri(m_textureID, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTextureParameteri(m_textureID, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTextureParameteri(m_textureID, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glNamedFramebufferTexture(m_fboID, GL_COLOR_ATTACHMENT0, m_textureID, 0);
		GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT0 };
		glNamedFramebufferDrawBuffers(m_fboID, 1, drawBuffers);
	}
	virtual void resize(const GLuint & width = 1, const GLuint & height = 1, const GLuint & depth = 1) {
		m_size = glm::ivec2(width, height);
		glTextureImage2DEXT(m_textureID, GL_TEXTURE_2D, 0, GL_RGB8, m_size.x, m_size.y, 0, GL_RGB, GL_FLOAT, NULL);
	}
	virtual void clear() {
		GLfloat clearColor[] = { 0.0f, 0.0f, 0.0f };
		glClearNamedFramebufferfv(m_fboID, GL_COLOR, 0, clearColor);
	}
	virtual void bindForWriting() {
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fboID);
	}
	virtual void bindForReading(const GLuint & binding = 0) {
		glBindTextureUnit(binding, m_textureID);
	}
	virtual void attachTexture(const GLuint & textureObj, const GLenum & attachPoint, const GLuint & level = 0) {
		glNamedFramebufferTexture(m_fboID, attachPoint, textureObj, level);
	}
};

#endif // FBO_REFLECTION_H