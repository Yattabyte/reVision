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

		glCreateTextures(GL_TEXTURE_2D, 1, &m_texture);
		glTextureImage2DEXT(m_texture, GL_TEXTURE_2D, 0, GL_RGB32F, m_renderSize.x, m_renderSize.y, 0, GL_RGB, GL_FLOAT, NULL);
		glTextureParameteri(m_texture, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTextureParameteri(m_texture, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTextureParameteri(m_texture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTextureParameteri(m_texture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glNamedFramebufferTexture(m_fbo, GL_COLOR_ATTACHMENT0, m_texture, 0);
		glNamedFramebufferDrawBuffer(m_fbo, GL_COLOR_ATTACHMENT0);
		validate();
	}
}

void Lighting_Buffer::bindForWriting()
{
	Frame_Buffer::bindForWriting();
	// Borrow the G_Buffer's depth-stencil texture
	glNamedFramebufferTexture(m_fbo, GL_DEPTH_STENCIL_ATTACHMENT, m_depth_stencil, 0);
}

void Lighting_Buffer::bindForReading()
{
	glBindTextureUnit(0, m_texture);
}

void Lighting_Buffer::resize(const ivec2 & size)
{
	Frame_Buffer::resize(size);

	glTextureImage2DEXT(m_texture, GL_TEXTURE_2D, 0, GL_RGB32F, m_renderSize.x, m_renderSize.y, 0, GL_RGB, GL_FLOAT, NULL);
	glNamedFramebufferTexture(m_fbo, GL_COLOR_ATTACHMENT0, m_texture, 0);
}

void Lighting_Buffer::end()
{
	// return the G_Buffer's depth-stencil texture
	glNamedFramebufferTexture(m_fbo, GL_DEPTH_STENCIL_ATTACHMENT, 0, 0);
}
