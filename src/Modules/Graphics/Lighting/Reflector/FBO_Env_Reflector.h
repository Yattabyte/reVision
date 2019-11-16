#pragma once
#ifndef FBO_ENVMAP_H
#define FBO_ENVMAP_H

#include "glm/glm.hpp"


/** Framebuffer containing texture data for all environment maps. */
class FBO_Env_Reflector {
public:
	// Public (De)Constructors
	/** Destroy this framebuffer. */
	inline ~FBO_Env_Reflector() noexcept {
		glDeleteFramebuffers(6, m_fboID);
		glDeleteTextures(1, &m_textureID);
	}
	/** Construct this framebuffer. */
	inline FBO_Env_Reflector() noexcept {
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
		resize(glm::ivec2(512), 6);
		glCreateFramebuffers(6, m_fboID);
		for (int x = 0; x < 6; ++x) {
			glNamedFramebufferTexture(m_fboID[x], GL_COLOR_ATTACHMENT0, m_textureID, x);
			glNamedFramebufferDrawBuffer(m_fboID[x], GL_COLOR_ATTACHMENT0);
		}
	}


	// Public Methods
	/** Set the size of this framebuffer.
	@param	newSize		the new size to use.
	@param	depth		the new depth to use. */
	inline void resize(const glm::ivec2 newSize, const GLuint& depth) noexcept {
		if (m_size != newSize || m_depth != depth) {
			m_size = newSize;
			m_depth = depth;
			for (int x = 0; x < 6; ++x) {
				const glm::ivec2 size(glm::floor(glm::vec2(m_size) / glm::vec2(powf(2.0f, (float)x))));
				glTextureImage3DEXT(m_textureID, GL_TEXTURE_CUBE_MAP_ARRAY, x, GL_RGB16F, size.x, size.y, m_depth, 0, GL_RGB, GL_FLOAT, nullptr);
			}
		}
	}
	/** Clear the data out of the entire framebuffer. */
	inline void clear() noexcept {
		GLfloat clearColor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
		for (int x = 0; x < 6; ++x)
			glClearNamedFramebufferfv(m_fboID[x], GL_COLOR, 0, clearColor);
	}
	/** Clear the data out of a specific layer in the framebuffer.
	@param	zOffset		the layer to clear out of. */
	inline void clear(const GLint& zOffset) noexcept {
		const glm::vec3 clear(0.0f);
		glClearTexSubImage(m_textureID, 0, 0, 0, zOffset, m_size.x, m_size.y, 6, GL_RGB, GL_FLOAT, &clear);
	}
	/** Bind this framebuffer for writing, at the specific mipmap index. */
	inline void bindForWriting(const int& index) noexcept {
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fboID[index]);
	}
	/** Bind this framebuffer for reading, with the binding offset specified.
	@param	binding		the texture binding point. */
	inline void bindForReading(const GLuint& binding = 0) noexcept {
		glBindTextureUnit(binding, m_textureID);
	}


private:
	// Private Attributes
	GLuint m_fboID[6] = { 0,0,0,0,0,0 }, m_textureID = 0;
	glm::ivec2 m_size = glm::ivec2(1);
	GLuint m_depth = 1;
};

#endif // FBO_ENVMAP_H
