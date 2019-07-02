#pragma once
#ifndef FBO_ENVMAP_H
#define FBO_ENVMAP_H 

#include "glm/glm.hpp"


/** A framebuffer, formatted for storing point light shadows (naive cubemap implementation). */
struct FBO_Env_Reflector {
	// Attributes
	GLuint m_fboID = 0, m_textureID = 0;
	glm::ivec2 m_size = glm::ivec2(1);


	// (de)Constructors
	/***/
	inline ~FBO_Env_Reflector() {
		glDeleteFramebuffers(1, &m_fboID);
		glDeleteTextures(1, &m_textureID);
	}
	/***/
	inline FBO_Env_Reflector() {
		glCreateFramebuffers(1, &m_fboID);
		glCreateTextures(GL_TEXTURE_CUBE_MAP_ARRAY, 1, &m_textureID);
		glTextureParameteri(m_textureID, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTextureParameteri(m_textureID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTextureParameteri(m_textureID, GL_TEXTURE_MIN_LOD, 0);
		glTextureParameteri(m_textureID, GL_TEXTURE_MAX_LOD, 5);
		glTextureParameteri(m_textureID, GL_TEXTURE_BASE_LEVEL, 0);
		glTextureParameteri(m_textureID, GL_TEXTURE_MAX_LEVEL, 5);
		glTextureParameteri(m_textureID, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTextureParameteri(m_textureID, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTextureParameteri(m_textureID, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		resize(512, 512, 6);
		glNamedFramebufferTexture(m_fboID, GL_COLOR_ATTACHMENT0, m_textureID, 0);
		glNamedFramebufferDrawBuffer(m_fboID, GL_COLOR_ATTACHMENT0);
	}


	// Methods
	/***/
	inline void resize(const GLuint & width = 1, const GLuint & height = 1, const GLuint & depth = 1) {
		m_size = glm::ivec2(width, height);
		for (int x = 0; x < 6; ++x) {
			const glm::ivec2 size(glm::floor(glm::vec2(m_size) / glm::vec2(powf(2.0f, (float)x))));
			glTextureImage3DEXT(m_textureID, GL_TEXTURE_CUBE_MAP_ARRAY, x, GL_RGB16F, size.x, size.y, depth, 0, GL_RGB, GL_FLOAT, NULL);
		}
	}
	/***/
	inline void clear() {
		GLfloat clearColor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
		glClearNamedFramebufferfv(m_fboID, GL_COLOR, 0, clearColor);
	}
	/***/
	inline void clear(const GLint & zOffset) {
		const glm::vec3 clear(0.0f);
		glClearTexSubImage(m_textureID, 0, 0, 0, zOffset, m_size.x, m_size.y, 6, GL_RGB, GL_FLOAT, &clear);
	}
	/***/
	inline void bindForWriting() {
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fboID);
	}
	/***/
	inline void bindForReading(const GLuint & binding = 0) {
		glBindTextureUnit(binding, m_textureID);
	}
	/***/
	inline void attachTexture(const GLuint & textureObj, const GLenum & attachPoint, const GLuint & level = 0) {
		glNamedFramebufferTexture(m_fboID, attachPoint, textureObj, level);
	}
};

#endif // FBO_ENVMAP_H