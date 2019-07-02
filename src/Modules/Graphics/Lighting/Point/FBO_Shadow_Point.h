#pragma once
#ifndef FBO_SHADOW_POINT_H
#define FBO_SHADOW_POINT_H 

#include "Utilities/GL/FBO.h"
#include "glm/glm.hpp"


/** A framebuffer, formatted for storing point light shadows (naive cubemap implementation). */
struct FBO_Shadow_Point {
	// Attributes
	GLuint m_fboID = 0, m_textureIDS[4] = { 0,0,0,0 };
	glm::ivec2 m_size = glm::ivec2(1);
	GLuint m_layerFaces = 6;


	// (de)Constructors
	inline ~FBO_Shadow_Point() {
		glDeleteFramebuffers(1, &m_fboID);
		glDeleteTextures(4, m_textureIDS);
	}
	inline FBO_Shadow_Point() {
		glCreateFramebuffers(1, &m_fboID);
		glCreateTextures(GL_TEXTURE_CUBE_MAP_ARRAY, 4, m_textureIDS);
		resize(glm::vec2(1), 6);
		glTextureParameteri(m_textureIDS[0], GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTextureParameteri(m_textureIDS[0], GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTextureParameteri(m_textureIDS[0], GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTextureParameteri(m_textureIDS[0], GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTextureParameteri(m_textureIDS[0], GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTextureParameteri(m_textureIDS[1], GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTextureParameteri(m_textureIDS[1], GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTextureParameteri(m_textureIDS[1], GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTextureParameteri(m_textureIDS[1], GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTextureParameteri(m_textureIDS[1], GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTextureParameteri(m_textureIDS[2], GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTextureParameteri(m_textureIDS[2], GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTextureParameteri(m_textureIDS[2], GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTextureParameteri(m_textureIDS[2], GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTextureParameteri(m_textureIDS[2], GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTextureParameteri(m_textureIDS[3], GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTextureParameteri(m_textureIDS[3], GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTextureParameteri(m_textureIDS[3], GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTextureParameteri(m_textureIDS[3], GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTextureParameteri(m_textureIDS[3], GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glNamedFramebufferTexture(m_fboID, GL_COLOR_ATTACHMENT0, m_textureIDS[0], 0);
		glNamedFramebufferTexture(m_fboID, GL_COLOR_ATTACHMENT1, m_textureIDS[1], 0);
		glNamedFramebufferTexture(m_fboID, GL_DEPTH_ATTACHMENT, m_textureIDS[2], 0);
		glNamedFramebufferTexture(m_fboID, GL_COLOR_ATTACHMENT2, m_textureIDS[3], 0);
		GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
		glNamedFramebufferDrawBuffers(m_fboID, 3, drawBuffers);
	}
	inline void resize(const glm::ivec2 & size, const unsigned int & layerFaces) {
		if (m_size.x != size.x || m_size.y != size.y || m_layerFaces != layerFaces) {
			m_size = size;
			m_layerFaces = layerFaces;
			const float clearDepth(1.0f);
			const glm::vec3 clear(0.0f);
			glTextureImage3DEXT(m_textureIDS[0], GL_TEXTURE_CUBE_MAP_ARRAY, 0, GL_RGB8, m_size.x, m_size.y, m_layerFaces, 0, GL_RGB, GL_FLOAT, NULL);
			glTextureImage3DEXT(m_textureIDS[1], GL_TEXTURE_CUBE_MAP_ARRAY, 0, GL_RGB8, m_size.x, m_size.y, m_layerFaces, 0, GL_RGB, GL_FLOAT, NULL);
			glTextureImage3DEXT(m_textureIDS[2], GL_TEXTURE_CUBE_MAP_ARRAY, 0, GL_DEPTH_COMPONENT, m_size.x, m_size.y, m_layerFaces, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
			glTextureImage3DEXT(m_textureIDS[3], GL_TEXTURE_CUBE_MAP_ARRAY, 0, GL_R16F, m_size.x, m_size.y, m_layerFaces, 0, GL_RED, GL_FLOAT, NULL);
			glClearTexImage(m_textureIDS[0], 0, GL_RGB, GL_FLOAT, &clear);
			glClearTexImage(m_textureIDS[1], 0, GL_RGB, GL_FLOAT, &clear);
			glClearTexImage(m_textureIDS[2], 0, GL_DEPTH_COMPONENT, GL_FLOAT, &clearDepth);
			glClearTexImage(m_textureIDS[3], 0, GL_RED, GL_FLOAT, &clear);
		}
	}
	inline void clear(const GLint & zOffset) {
		const float clearDepth(1.0f);
		const glm::vec3 clear(0.0f);
		glClearTexSubImage(m_textureIDS[0], 0, 0, 0, zOffset, m_size.x, m_size.y, 6, GL_RGB, GL_FLOAT, &clear);
		glClearTexSubImage(m_textureIDS[1], 0, 0, 0, zOffset, m_size.x, m_size.y, 6, GL_RGB, GL_FLOAT, &clear);
		glClearTexSubImage(m_textureIDS[2], 0, 0, 0, zOffset, m_size.x, m_size.y, 6, GL_DEPTH_COMPONENT, GL_FLOAT, &clearDepth);
		glClearTexSubImage(m_textureIDS[3], 0, 0, 0, zOffset, m_size.x, m_size.y, 6, GL_RED, GL_FLOAT, &clear);
	}
	inline void bindForWriting() {
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fboID);
	}
};

#endif // FBO_SHADOW_POINT_H