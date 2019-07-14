#pragma once
#ifndef FBO_ENVMAP_H
#define FBO_ENVMAP_H 

#include "glm/glm.hpp"


/***/
struct FBO_Env_Reflector {
	// Attributes
	GLuint m_fboID[6] = { 0,0,0,0,0,0 }, m_textureID = 0;
	glm::ivec2 m_size = glm::ivec2(1);
	int m_depth = 1;


	// (de)Constructors
	/***/
	inline ~FBO_Env_Reflector() {
		glDeleteFramebuffers(6, m_fboID);
		glDeleteTextures(1, &m_textureID);
	}
	/***/
	inline FBO_Env_Reflector() {
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
		glCreateFramebuffers(6, m_fboID);
		for (int x = 0; x < 6; ++x) {
			glNamedFramebufferTexture(m_fboID[x], GL_COLOR_ATTACHMENT0, m_textureID, x);
			glNamedFramebufferDrawBuffer(m_fboID[x], GL_COLOR_ATTACHMENT0);
		}
	}


	// Methods
	/***/
	inline void resize(const GLuint & width = 1, const GLuint & height = 1, const GLuint & depth = 1) {
		if (m_size.x != width || m_size.y != height || m_depth != depth) {
			m_size = glm::ivec2(width, height);
			m_depth = depth;
			for (int x = 0; x < 6; ++x) {
				const glm::ivec2 size(glm::floor(glm::vec2(m_size) / glm::vec2(powf(2.0f, (float)x))));
				glTextureImage3DEXT(m_textureID, GL_TEXTURE_CUBE_MAP_ARRAY, x, GL_RGB16F, size.x, size.y, depth, 0, GL_RGB, GL_FLOAT, NULL);
			}
		}
	}
	/***/
	inline void clear() {
		GLfloat clearColor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
		for (int x = 0; x < 6; ++x) 
			glClearNamedFramebufferfv(m_fboID[x], GL_COLOR, 0, clearColor);
	}
	/***/
	inline void clear(const GLint & zOffset) {
		const glm::vec3 clear(0.0f);
		glClearTexSubImage(m_textureID, 0, 0, 0, zOffset, m_size.x, m_size.y, 6, GL_RGB, GL_FLOAT, &clear);
	}
	/***/
	inline void bindForWriting(const int & index) {
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fboID[index]);
	}
	/***/
	inline void bindForReading(const GLuint & binding = 0) {
		glBindTextureUnit(binding, m_textureID);
	}
};

#endif // FBO_ENVMAP_H