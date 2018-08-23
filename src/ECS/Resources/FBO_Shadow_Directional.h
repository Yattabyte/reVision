#pragma once
#ifndef FBO_SHADOW_DIRECTIONAL_H
#define FBO_SHADOW_DIRECTIONAL_H 

#include "Utilities\GL\FBO.h"
#include "GLFW\glfw3.h"


/** A framebuffer, formatted for storing directional light shadows, 4 at a time (parallel split cascaded shadow maps). */
struct FBO_Shadow_Directional {
	GLuint m_fboID = 0, m_textures[3];
	glm::ivec2 m_size;
	FBO_Shadow_Directional() {
		glCreateFramebuffers(1, &m_fboID);
		glCreateTextures(GL_TEXTURE_2D_ARRAY, 3, m_textures);
		resize(glm::vec2(1), 4);
		glTextureParameteri(m_textures[0], GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTextureParameteri(m_textures[0], GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTextureParameteri(m_textures[0], GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTextureParameteri(m_textures[0], GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTextureParameteri(m_textures[1], GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTextureParameteri(m_textures[1], GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTextureParameteri(m_textures[1], GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTextureParameteri(m_textures[1], GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTextureParameteri(m_textures[2], GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTextureParameteri(m_textures[2], GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTextureParameteri(m_textures[2], GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureParameteri(m_textures[2], GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTextureParameteri(m_textures[2], GL_DEPTH_TEXTURE_MODE, GL_INTENSITY);
		glNamedFramebufferTexture(m_fboID, GL_COLOR_ATTACHMENT0, m_textures[0], 0);
		glNamedFramebufferTexture(m_fboID, GL_COLOR_ATTACHMENT1, m_textures[1], 0);
		glNamedFramebufferTexture(m_fboID, GL_DEPTH_ATTACHMENT, m_textures[2], 0);
		GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
		glNamedFramebufferDrawBuffers(m_fboID, 2, drawBuffers);
	}
	void resize(const glm::ivec2 & size, const unsigned int & layerFaces) {
		m_size = size;
		glTextureImage3DEXT(m_textures[0], GL_TEXTURE_2D_ARRAY, 0, GL_RGB8, m_size.x, m_size.y, layerFaces, 0, GL_RGB, GL_FLOAT, NULL);
		glTextureImage3DEXT(m_textures[1], GL_TEXTURE_2D_ARRAY, 0, GL_RGB8, m_size.x, m_size.y, layerFaces, 0, GL_RGB, GL_FLOAT, NULL);
		glTextureImage3DEXT(m_textures[2], GL_TEXTURE_2D_ARRAY, 0, GL_DEPTH_COMPONENT, m_size.x, m_size.y, layerFaces, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	}
	void clear(const GLint & zOffset) {
		const float clearDepth(1.0f);
		const glm::vec3 clear(0.0f);
		glClearTexSubImage(m_textures[0], 0, 0, 0, zOffset, m_size.x, m_size.y, 4, GL_RGB, GL_FLOAT, &clear);
		glClearTexSubImage(m_textures[1], 0, 0, 0, zOffset, m_size.x, m_size.y, 4, GL_RGB, GL_FLOAT, &clear);
		glClearTexSubImage(m_textures[2], 0, 0, 0, zOffset, m_size.x, m_size.y, 4, GL_DEPTH_COMPONENT, GL_FLOAT, &clearDepth);
	}
	void bindForWriting() {
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fboID);
	}
};

#endif // FBO_SHADOW_DIRECTIONAL_H