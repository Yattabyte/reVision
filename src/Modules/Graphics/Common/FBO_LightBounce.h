#pragma once
#ifndef FBO_LIGHTBOUNCE_H
#define FBO_LIGHTBOUNCE_H 
#define RH_TEXTURE_COUNT 4

#include "Utilities\GL\FBO.h"
#include "GLFW\glfw3.h"


/** A framebuffer, formatted for rendering radiance hints (indirect radiant diffuse lighting). */
struct FBO_LightBounce : FBO_Base {
	GLuint m_fboID = 0, m_textureIDS[RH_TEXTURE_COUNT] = { 0,0,0,0 };
	GLuint m_resolution = 1;
	~FBO_LightBounce() {
		glDeleteFramebuffers(1, &m_fboID);
		glDeleteTextures(RH_TEXTURE_COUNT, m_textureIDS);
	}
	FBO_LightBounce() {
		glCreateFramebuffers(1, &m_fboID);
		glCreateTextures(GL_TEXTURE_3D, RH_TEXTURE_COUNT, m_textureIDS);
		for (int channel = 0; channel < RH_TEXTURE_COUNT; ++channel) {
			glTextureParameteri(m_textureIDS[channel], GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTextureParameteri(m_textureIDS[channel], GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTextureParameteri(m_textureIDS[channel], GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
			glTextureParameteri(m_textureIDS[channel], GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTextureParameteri(m_textureIDS[channel], GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glNamedFramebufferTexture(m_fboID, GL_COLOR_ATTACHMENT0 + channel, m_textureIDS[channel], 0);
		}
		const GLenum Buffers[] = { 
			GL_COLOR_ATTACHMENT0,
			GL_COLOR_ATTACHMENT1,
			GL_COLOR_ATTACHMENT2,
			GL_COLOR_ATTACHMENT3 
		};
		glNamedFramebufferDrawBuffers(m_fboID, 4, Buffers);
		
		resize();
	}
	virtual void resize(const GLuint & width = 1, const GLuint & height = 1, const GLuint & depth = 1) override {
		m_resolution = width;
		for (int channel = 0; channel < RH_TEXTURE_COUNT; ++channel)
			glTextureImage3DEXT(m_textureIDS[channel], GL_TEXTURE_3D, 0, GL_RGBA16F, m_resolution, m_resolution, m_resolution, 0, GL_RGBA, GL_FLOAT, 0);
	}
	virtual void clear() override {
		GLfloat clearColor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
		for (GLint x = 0; x < RH_TEXTURE_COUNT; ++x)
			glClearNamedFramebufferfv(m_fboID, GL_COLOR, x, clearColor);
	}
	virtual void bindForWriting() override {
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fboID);
	}
	virtual void bindForReading(const GLuint & binding = 0) override {
		for (GLuint x = 0; x < RH_TEXTURE_COUNT; ++x)
			glBindTextureUnit(binding + x, m_textureIDS[x]);
	}
	virtual void attachTexture(const GLuint & textureObj, const GLenum & attachPoint, const GLuint & level = 0) override {
		glNamedFramebufferTexture(m_fboID, attachPoint, textureObj, level);
	}
};

#endif // FBO_LIGHTBOUNCE_H