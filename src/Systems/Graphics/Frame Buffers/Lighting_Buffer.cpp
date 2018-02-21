#include "Systems\Graphics\Frame Buffers\Lighting_Buffer.h"
#include "Systems\Graphics\VisualFX.h"
#include "Managers\Message_Manager.h"
#include <algorithm>


Lighting_Buffer::~Lighting_Buffer()
{
	if (m_Initialized) {
		// Destroy OpenGL objects
		glDeleteTextures(LBUFFER_NUM_TEXTURES, m_textures);
		glDeleteTextures(2, m_texturesGB);
		glDeleteFramebuffers(1, &m_fbo);
	}
}

Lighting_Buffer::Lighting_Buffer()
{
	m_Initialized = false;
	m_depth_stencil = 0;
	m_fbo = 0;
	m_renderSize = vec2(512);
	m_bloomStrength = 8;
	for (int x = 0; x < LBUFFER_NUM_TEXTURES; ++x)
		m_textures[x] = 0;
	for (int x = 0; x < 2; ++x)
		m_texturesGB[x] = 0;
}

void Lighting_Buffer::initialize(const vec2 & size, VisualFX * visualFX, const int & bloomStrength, const GLuint & depthStencil)
{
	if (!m_Initialized) {
		m_depth_stencil = depthStencil;
		m_visualFX = visualFX;
		m_renderSize = size;
		m_bloomStrength = bloomStrength;

		glGenFramebuffers(1, &m_fbo);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo);
		glGenTextures(LBUFFER_NUM_TEXTURES, m_textures);
		for (int x = 0; x < LBUFFER_NUM_TEXTURES; ++x) {
			glBindTexture(GL_TEXTURE_2D, m_textures[x]);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, m_renderSize.x, m_renderSize.y, 0, GL_RGB, GL_FLOAT, NULL);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + x, GL_TEXTURE_2D, m_textures[x], 0);
		}
		glGenTextures(2, m_texturesGB);
		for (int x = 0; x < 2; ++x) {
			glBindTexture(GL_TEXTURE_2D, m_texturesGB[x]);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, m_renderSize.x, m_renderSize.y, 0, GL_RGB, GL_FLOAT, NULL);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		}

		GLenum Status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		if (Status != GL_FRAMEBUFFER_COMPLETE && Status != GL_NO_ERROR) {
			std::string errorString = std::string(reinterpret_cast<char const *>(glewGetErrorString(Status)));
			MSG_Manager::Error(MSG_Manager::FBO_INCOMPLETE, "Lighting Buffer", errorString);
			return;
		}
		m_Initialized = true;
	}
}

void Lighting_Buffer::clear()
{
	GLenum DrawBuffers[] = {
		GL_COLOR_ATTACHMENT0,
		GL_COLOR_ATTACHMENT1
	};

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo);
	glDrawBuffers(LBUFFER_NUM_TEXTURES, DrawBuffers);
	glClear(GL_COLOR_BUFFER_BIT);
}

void Lighting_Buffer::bindForWriting()
{
	GLenum DrawBuffers[] = {
		GL_COLOR_ATTACHMENT0,
		GL_COLOR_ATTACHMENT1
	};

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo);
	// Borrow the G_Buffer's depth-stencil texture
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, m_depth_stencil, 0);
	glDrawBuffers(LBUFFER_NUM_TEXTURES, DrawBuffers);
}

void Lighting_Buffer::bindForReading()
{
	for (unsigned int i = 0; i < LBUFFER_NUM_TEXTURES; i++) {
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, m_textures[i]);
	}
}

void Lighting_Buffer::resize(const vec2 & size)
{
	m_renderSize = size;
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo);

	for (int x = 0; x < LBUFFER_NUM_TEXTURES; ++x) {
		glBindTexture(GL_TEXTURE_2D, m_textures[x]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, size.x, size.y, 0, GL_RGB, GL_FLOAT, NULL);
	}
	for (int x = 0; x < 2; ++x) {
		glBindTexture(GL_TEXTURE_2D, m_texturesGB[x]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, size.x, size.y, 0, GL_RGB, GL_FLOAT, NULL);
	}

	// restore default FBO
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}

void Lighting_Buffer::setBloomStrength(const int & strength)
{
	m_bloomStrength = strength;
}

void Lighting_Buffer::applyBloom()
{
	glDisable(GL_BLEND);
	m_visualFX->applyGaussianBlur(m_textures[LBUFFER_TEXTURE_TYPE_OVERBRIGHT], m_texturesGB, m_renderSize, m_bloomStrength);

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo);
	// return the G_Buffer's depth-stencil texture
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, 0, 0);
	// return our overbright texture following its convolution
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + LBUFFER_TEXTURE_TYPE_OVERBRIGHT, GL_TEXTURE_2D, m_textures[LBUFFER_TEXTURE_TYPE_OVERBRIGHT], 0);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}
