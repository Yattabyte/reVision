#pragma once
#ifndef FBO_SHADOW_SPOT_H
#define FBO_SHADOW_SPOT_H 

#include "Utilities\GL\FBO.h"
#include "GLFW\glfw3.h"


/** A framebuffer, formatted for storing spot light shadows. */
struct FBO_Shadow_Spot {
	GLuint m_fboID = 0, m_textureIDS[3] = { 0,0,0 };
	glm::ivec2 m_size = glm::ivec2(1);
	~FBO_Shadow_Spot() {
		glDeleteFramebuffers(1, &m_fboID);
		glDeleteTextures(3, m_textureIDS);
	}
	FBO_Shadow_Spot() {
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
	inline void resize(const glm::ivec2 & size, const unsigned int & layerFaces) {
		m_size = size;
		glTextureImage3DEXT(m_textureIDS[0], GL_TEXTURE_2D_ARRAY, 0, GL_RGB8, m_size.x, m_size.y, layerFaces, 0, GL_RGB, GL_FLOAT, NULL);
		glTextureImage3DEXT(m_textureIDS[1], GL_TEXTURE_2D_ARRAY, 0, GL_RGB8, m_size.x, m_size.y, layerFaces, 0, GL_RGB, GL_FLOAT, NULL);
		glTextureImage3DEXT(m_textureIDS[2], GL_TEXTURE_2D_ARRAY, 0, GL_DEPTH_COMPONENT, m_size.x, m_size.y, layerFaces, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	}
	inline void clear(const GLint & zOffset) {
		const float clearDepth(1.0f);
		const glm::vec3 clear(0.0f);
		glClearTexSubImage(m_textureIDS[0], 0, 0, 0, zOffset, m_size.x, m_size.y, 1, GL_RGB, GL_FLOAT, &clear);
		glClearTexSubImage(m_textureIDS[1], 0, 0, 0, zOffset, m_size.x, m_size.y, 1, GL_RGB, GL_FLOAT, &clear);
		glClearTexSubImage(m_textureIDS[2], 0, 0, 0, zOffset, m_size.x, m_size.y, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &clearDepth);
	}
	inline void bindForWriting() {
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fboID);
	}
};

#endif // FBO_SHADOW_SPOT_H