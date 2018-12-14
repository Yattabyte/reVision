#pragma once
#ifndef FBO_GEOMETRY_H
#define FBO_GEOMETRY_H 

#include "Utilities\GL\FBO.h"
#include "GLFW\glfw3.h"
#include "glm\glm.hpp"


/** A framebuffer, formatted to support rendering geometry (deferred rendering/gbuffer). */
struct FBO_Geometry : FBO_Base {
	GLuint m_fboID = 0, m_textureIDS[4] = { 0,0,0,0 };
	glm::ivec2 m_size = glm::ivec2(1);
	~FBO_Geometry() {
		glDeleteFramebuffers(1, &m_fboID);
		glDeleteTextures(4, m_textureIDS);
	}
	FBO_Geometry() {
		glCreateFramebuffers(1, &m_fboID);
		glCreateTextures(GL_TEXTURE_2D, 4, m_textureIDS);
		resize();
		glTextureParameteri(m_textureIDS[0], GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTextureParameteri(m_textureIDS[0], GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTextureParameteri(m_textureIDS[0], GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTextureParameteri(m_textureIDS[0], GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTextureParameteri(m_textureIDS[1], GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTextureParameteri(m_textureIDS[1], GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTextureParameteri(m_textureIDS[1], GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTextureParameteri(m_textureIDS[1], GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTextureParameteri(m_textureIDS[2], GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTextureParameteri(m_textureIDS[2], GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTextureParameteri(m_textureIDS[2], GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTextureParameteri(m_textureIDS[2], GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTextureParameteri(m_textureIDS[3], GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTextureParameteri(m_textureIDS[3], GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTextureParameteri(m_textureIDS[3], GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTextureParameteri(m_textureIDS[3], GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glNamedFramebufferTexture(m_fboID, GL_COLOR_ATTACHMENT0, m_textureIDS[0], 0);
		glNamedFramebufferTexture(m_fboID, GL_COLOR_ATTACHMENT1, m_textureIDS[1], 0);
		glNamedFramebufferTexture(m_fboID, GL_COLOR_ATTACHMENT2, m_textureIDS[2], 0);
		glNamedFramebufferTexture(m_fboID, GL_DEPTH_STENCIL_ATTACHMENT, m_textureIDS[3], 0);
		GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
		glNamedFramebufferDrawBuffers(m_fboID, 3, drawBuffers);
	}
	// Interface Implementation
	inline virtual void resize(const GLuint & width = 1, const GLuint & height = 1, const GLuint & depth = 1) override {
		m_size = glm::ivec2(width, height);
		glTextureImage2DEXT(m_textureIDS[0], GL_TEXTURE_2D, 0, GL_RGB16F, m_size.x, m_size.y, 0, GL_RGB, GL_FLOAT, NULL);
		glTextureImage2DEXT(m_textureIDS[1], GL_TEXTURE_2D, 0, GL_RGB16F, m_size.x, m_size.y, 0, GL_RGB, GL_FLOAT, NULL);
		glTextureImage2DEXT(m_textureIDS[2], GL_TEXTURE_2D, 0, GL_RGBA16F, m_size.x, m_size.y, 0, GL_RGBA, GL_FLOAT, NULL);
		glTextureImage2DEXT(m_textureIDS[3], GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, m_size.x, m_size.y, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);
	}
	inline virtual void clear() override {
		GLfloat clearColor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
		GLfloat clearDepth = 1.0f;
		GLint clearStencil = 0;
		for (int x = 0; x < 4; ++x)
			glClearNamedFramebufferfv(m_fboID, GL_COLOR, x, clearColor);
		glClearNamedFramebufferfi(m_fboID, GL_DEPTH_STENCIL, 0, clearDepth, clearStencil);
	}
	inline virtual void bindForWriting() override {
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fboID);
	}
	inline virtual void bindForReading(const GLuint & binding = 0) override {
		for (int x = 0; x < 4; ++x)
		glBindTextureUnit(x + binding, m_textureIDS[x]);
	}
	inline virtual void attachTexture(const GLuint & textureObj, const GLenum & attachPoint, const GLuint & level = 0) override {
		glNamedFramebufferTexture(m_fboID, attachPoint, textureObj, level);
	}
};

#endif // FBO_GEOMETRY_H