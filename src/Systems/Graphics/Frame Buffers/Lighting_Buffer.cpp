#include "Systems\Graphics\Frame Buffers\Lighting_Buffer.h"
#include "Utilities\EnginePackage.h"


Lighting_Buffer::~Lighting_Buffer()
{
	if (m_Initialized) {
		// Destroy OpenGL objects
		glDeleteTextures(1, &m_texture);
		m_enginePackage->removePrefCallback(PreferenceState::C_WINDOW_WIDTH, this);
		m_enginePackage->removePrefCallback(PreferenceState::C_WINDOW_HEIGHT, this);
	}
}

Lighting_Buffer::Lighting_Buffer()
{\
	m_depth_stencil = 0;
	m_texture = 0;
}

void Lighting_Buffer::initialize(EnginePackage * enginePackage, const GLuint & depthStencil)
{
	if (!m_Initialized) {
		m_enginePackage = enginePackage;
		m_depth_stencil = depthStencil;
		m_renderSize.x = m_enginePackage->addPrefCallback(PreferenceState::C_WINDOW_WIDTH, this, [&](const float &f) {resize(ivec2(f, m_renderSize.y)); });
		m_renderSize.y = m_enginePackage->addPrefCallback(PreferenceState::C_WINDOW_HEIGHT, this, [&](const float &f) {resize(ivec2(m_renderSize.x, f)); });
		Frame_Buffer::initialize();

		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo);
		glGenTextures(1, &m_texture);
		glBindTexture(GL_TEXTURE_2D, m_texture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, m_renderSize.x, m_renderSize.y, 0, GL_RGB, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_texture, 0);
		glDrawBuffer(GL_COLOR_ATTACHMENT0);		
		validate();
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
}

void Lighting_Buffer::bindForWriting()
{
	Frame_Buffer::bindForWriting();
	// Borrow the G_Buffer's depth-stencil texture
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, m_depth_stencil, 0);
}

void Lighting_Buffer::bindForReading()
{
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_texture);	
}

void Lighting_Buffer::resize(const ivec2 & size)
{
	Frame_Buffer::resize(size);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo);

	glBindTexture(GL_TEXTURE_2D, m_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, m_renderSize.x, m_renderSize.y, 0, GL_RGB, GL_FLOAT, NULL);
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_texture, 0);

	// restore default FBO
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}

void Lighting_Buffer::end()
{
	// return the G_Buffer's depth-stencil texture
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo);
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, 0, 0);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}
