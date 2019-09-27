#pragma once
#ifndef SHADOW_MAP_H
#define SHADOW_MAP_H

#include "Utilities/GL/glad/glad.h"
#include "glm/glm.hpp"


/** Framebuffer containing shadow data for all light maps. */
class ShadowMap {
public:
	// Public (de)Constructors
	/** Destroy this framebuffer. */
	inline ~ShadowMap() {
		glDeleteFramebuffers(1, &m_fboID);
		glDeleteTextures(3, m_textureIDS);
	}
	/** Construct this framebuffer. */
	inline ShadowMap() {
		glCreateFramebuffers(1, &m_fboID);
		glCreateTextures(GL_TEXTURE_2D_ARRAY, 3, m_textureIDS);
		resize(glm::vec2(1), 1);
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
		glNamedFramebufferTexture(m_fboID, GL_COLOR_ATTACHMENT0, m_textureIDS[0], 0);
		glNamedFramebufferTexture(m_fboID, GL_COLOR_ATTACHMENT1, m_textureIDS[1], 0);
		glNamedFramebufferTexture(m_fboID, GL_DEPTH_ATTACHMENT, m_textureIDS[2], 0);
		GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
		glNamedFramebufferDrawBuffers(m_fboID, 2, drawBuffers);
	}


	// Public Methods
	/** Set the size of this framebuffer.
	@param	newSize		the new size to use.
	@param	depth		the new depth to use. */
	inline void resize(const glm::ivec2& newSize, const int& layerFaces) {
		if (m_size != newSize || m_layerFaces != layerFaces) {
			m_size = glm::max(glm::ivec2(1), newSize);
			m_layerFaces = std::max<int>(1, layerFaces);
			constexpr float clearDepth(1.0f);
			constexpr glm::vec3 clear(0.0f);
			glTextureImage3DEXT(m_textureIDS[0], GL_TEXTURE_2D_ARRAY, 0, GL_RGB8, m_size.x, m_size.y, m_layerFaces, 0, GL_RGB, GL_FLOAT, NULL);
			glTextureImage3DEXT(m_textureIDS[1], GL_TEXTURE_2D_ARRAY, 0, GL_RGB8, m_size.x, m_size.y, m_layerFaces, 0, GL_RGB, GL_FLOAT, NULL);
			glTextureImage3DEXT(m_textureIDS[2], GL_TEXTURE_2D_ARRAY, 0, GL_DEPTH_COMPONENT, m_size.x, m_size.y, m_layerFaces, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
			glClearTexImage(m_textureIDS[0], 0, GL_RGB, GL_FLOAT, &clear);
			glClearTexImage(m_textureIDS[1], 0, GL_RGB, GL_FLOAT, &clear);
			glClearTexImage(m_textureIDS[2], 0, GL_DEPTH_COMPONENT, GL_FLOAT, &clearDepth);
		}
	}
	/** Clear the data out of a specific layer in the framebuffer.
	@param	zOffset		the layer to clear out of.
	@param	amount		the number of layers to clear. */
	inline void clear(const GLint& zOffset, const GLsizei& amount) {
		constexpr float clearDepth(1.0f);
		constexpr glm::vec3 clear(0.0f);
		glClearTexSubImage(m_textureIDS[0], 0, 0, 0, zOffset, m_size.x, m_size.y, amount, GL_RGB, GL_FLOAT, &clear);
		glClearTexSubImage(m_textureIDS[1], 0, 0, 0, zOffset, m_size.x, m_size.y, amount, GL_RGB, GL_FLOAT, &clear);
		glClearTexSubImage(m_textureIDS[2], 0, 0, 0, zOffset, m_size.x, m_size.y, amount, GL_DEPTH_COMPONENT, GL_FLOAT, &clearDepth);
	}
	/** Bind this framebuffer for writting. */
	inline void bindForWriting() {
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fboID);
	}


	// Public Attributes
	const GLuint& m_texNormal = m_textureIDS[0];
	const GLuint& m_texColor = m_textureIDS[1];
	const GLuint& m_texDepth = m_textureIDS[2];


private:
	// Private Attributes
	GLuint m_fboID = 0, m_textureIDS[3] = { 0,0,0 };
	glm::ivec2 m_size = glm::ivec2(1);
	int m_layerFaces = 1;
};

#endif // SHADOW_MAP_H