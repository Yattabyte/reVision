#include "Systems\Graphics\HDR_Buffer.h"
#include "Managers\Message_Manager.h"
#include <algorithm>


HDR_Buffer::~HDR_Buffer()
{
	if (m_Initialized) {
		// Destroy OpenGL objects
		glDeleteTextures(1, &m_texture);
		glDeleteFramebuffers(1, &m_fbo);
	}
}

HDR_Buffer::HDR_Buffer()
{
	m_Initialized = false;
	m_fbo = 0;
	m_texture = 0;
}

void HDR_Buffer::initialize(const vec2 & size)
{
	if (!m_Initialized) {
		glGenFramebuffers(1, &m_fbo);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo);
		glGenTextures(1, &m_texture);
		glBindTexture(GL_TEXTURE_2D, m_texture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, size.x, size.y, 0, GL_RGB, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_texture, 0);
		glDrawBuffer(GL_COLOR_ATTACHMENT0);

		GLenum Status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		if (Status != GL_FRAMEBUFFER_COMPLETE && Status != GL_NO_ERROR) {
			std::string errorString = std::string(reinterpret_cast<char const *>(glewGetErrorString(Status)));
			MSG_Manager::Error(MSG_Manager::FBO_INCOMPLETE, "Lighting Buffer", errorString);
			return;
		}
		m_Initialized = true;
	}
}

void HDR_Buffer::clear()
{
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo);
	glClear(GL_COLOR_BUFFER_BIT);
}

void HDR_Buffer::bindForWriting()
{
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo);
}

void HDR_Buffer::bindForReading()
{
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_texture);	
}

void HDR_Buffer::resize(const vec2 & size)
{
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo);

	glBindTexture(GL_TEXTURE_2D, m_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, size.x, size.y, 0, GL_RGB, GL_FLOAT, NULL);

	// restore default FBO
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}
