#include "Modules/Graphics/Lighting/Shadow/ShadowMap.h"
#include <algorithm>


ShadowMap::~ShadowMap() noexcept 
{
	glDeleteFramebuffers(1, &m_fboID);
	glDeleteTextures(3, m_textureIDS);
}

ShadowMap::ShadowMap() noexcept 
{
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
	constexpr GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
	glNamedFramebufferDrawBuffers(m_fboID, 2, drawBuffers);
}

void ShadowMap::resize(const glm::ivec2& newSize, const int& depth) noexcept 
{
	if (m_size != newSize || m_layerFaces != depth) {
		m_size = glm::max(glm::ivec2(1), newSize);
		m_layerFaces = std::max<int>(1, depth);
		constexpr float clearDepth(1.0f);
		constexpr glm::vec3 clearColor(0.0f);
		glTextureImage3DEXT(m_textureIDS[0], GL_TEXTURE_2D_ARRAY, 0, GL_RGB8, m_size.x, m_size.y, m_layerFaces, 0, GL_RGB, GL_FLOAT, nullptr);
		glTextureImage3DEXT(m_textureIDS[1], GL_TEXTURE_2D_ARRAY, 0, GL_RGB8, m_size.x, m_size.y, m_layerFaces, 0, GL_RGB, GL_FLOAT, nullptr);
		glTextureImage3DEXT(m_textureIDS[2], GL_TEXTURE_2D_ARRAY, 0, GL_DEPTH_COMPONENT, m_size.x, m_size.y, m_layerFaces, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
		glClearTexImage(m_textureIDS[0], 0, GL_RGB, GL_FLOAT, &clearColor);
		glClearTexImage(m_textureIDS[1], 0, GL_RGB, GL_FLOAT, &clearColor);
		glClearTexImage(m_textureIDS[2], 0, GL_DEPTH_COMPONENT, GL_FLOAT, &clearDepth);
	}
}

void ShadowMap::clear(const GLint& zOffset, const GLsizei& amount) noexcept 
{
	constexpr float clearDepth(1.0f);
	constexpr glm::vec3 clear(0.0f);
	glClearTexSubImage(m_textureIDS[0], 0, 0, 0, zOffset, m_size.x, m_size.y, amount, GL_RGB, GL_FLOAT, &clear);
	glClearTexSubImage(m_textureIDS[1], 0, 0, 0, zOffset, m_size.x, m_size.y, amount, GL_RGB, GL_FLOAT, &clear);
	glClearTexSubImage(m_textureIDS[2], 0, 0, 0, zOffset, m_size.x, m_size.y, amount, GL_DEPTH_COMPONENT, GL_FLOAT, &clearDepth);
}

void ShadowMap::bindForWriting() noexcept 
{
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fboID);
}