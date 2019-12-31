#include "Modules/Graphics/Lighting/Reflector/FBO_Env_Reflector.h"


FBO_Env_Reflector::~FBO_Env_Reflector()
{
	glDeleteFramebuffers(6, m_fboID);
	glDeleteTextures(1, &m_textureID);
}

FBO_Env_Reflector::FBO_Env_Reflector() noexcept
{
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

void FBO_Env_Reflector::resize(const glm::ivec2 newSize, const GLuint& depth)
{
	if (m_size != newSize || m_depth != depth) {
		m_size = newSize;
		m_depth = depth;
		for (int x = 0; x < 6; ++x) {
			const glm::ivec2 size(glm::floor(glm::vec2(m_size) / glm::vec2(powf(2.0F, static_cast<float>(x)))));
			glTextureImage3DEXT(m_textureID, GL_TEXTURE_CUBE_MAP_ARRAY, x, GL_RGB16F, size.x, size.y, m_depth, 0, GL_RGB, GL_FLOAT, nullptr);
		}
	}
}

void FBO_Env_Reflector::clear() noexcept
{
	constexpr GLfloat clearColor[4] = { 0.0F, 0.0F, 0.0F, 0.0F };
	for (int x = 0; x < 6; ++x)
		glClearNamedFramebufferfv(m_fboID[x], GL_COLOR, 0, clearColor);
}

void FBO_Env_Reflector::clear(const GLint& zOffset) noexcept
{
	constexpr static const glm::vec3 clear(0.0F);
	glClearTexSubImage(m_textureID, 0, 0, 0, zOffset, m_size.x, m_size.y, 6, GL_RGB, GL_FLOAT, &clear);
}

void FBO_Env_Reflector::bindForWriting(const int& index) noexcept
{
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fboID[index]);
}

void FBO_Env_Reflector::bindForReading(const GLuint& binding) noexcept
{
	glBindTextureUnit(binding, m_textureID);
}