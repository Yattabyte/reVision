#pragma once
#ifndef FBO_LIGHTING_H
#define FBO_LIGHTING_H 

#include "Utilities\GL\FBO.h"
#include "GLFW\glfw3.h"


/** A framebuffer, formatted for rendering lighting (deferred, using volumetric lights). */
struct FBO_Lighting : FBO_Base {
	GLuint m_fboID = 0, m_textureID = 0;
	glm::ivec2 m_size = glm::ivec2(1);
	~FBO_Lighting() {
		glDeleteFramebuffers(1, &m_fboID);
		glDeleteTextures(1, &m_textureID);
	}
	FBO_Lighting() {
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
	// Interface Implementation
	inline virtual void resize(const GLuint & width = 1, const GLuint & height = 1, const GLuint & depth = 1) override {
		m_size = glm::ivec2(width, height);
		glTextureImage2DEXT(m_textureID, GL_TEXTURE_2D, 0, GL_RGB16F, m_size.x, m_size.y, 0, GL_RGB, GL_FLOAT, NULL);
	}
	inline virtual void clear() override {
		GLfloat clearColor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
		glClearNamedFramebufferfv(m_fboID, GL_COLOR, 0, clearColor);
	}
	inline virtual void bindForWriting() override {
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fboID);
	}
	inline virtual void bindForReading(const GLuint & binding = 0) override {
		glBindTextureUnit(binding, m_textureID);
	}
	inline virtual void attachTexture(const GLuint & textureObj, const GLenum & attachPoint, const GLuint & level = 0) override {
		glNamedFramebufferTexture(m_fboID, attachPoint, textureObj, level);
	}
};

#endif // FBO_LIGHTING_H