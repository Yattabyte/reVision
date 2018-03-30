#include "Systems\Graphics\Resources\Frame Buffers\Reflection_FBO.h"
#include "Utilities\EnginePackage.h"


Reflection_FBO::~Reflection_FBO()
{
	if (m_Initialized) {
		if (m_texture != 0) glDeleteTextures(1, &m_texture);
		m_enginePackage->removePrefCallback(PreferenceState::C_WINDOW_WIDTH, this);
		m_enginePackage->removePrefCallback(PreferenceState::C_WINDOW_HEIGHT, this);
	}
}

Reflection_FBO::Reflection_FBO()
{
	m_texture = 0;
}

void Reflection_FBO::initialize(EnginePackage * enginePackage, const GLuint & depthStencil)
{
	if (!m_Initialized) {
		m_enginePackage = enginePackage;
		m_depth_stencil = depthStencil;
		m_renderSize.x = m_enginePackage->addPrefCallback(PreferenceState::C_WINDOW_WIDTH, this, [&](const float &f) {resize(ivec2(f, m_renderSize.y)); });
		m_renderSize.y = m_enginePackage->addPrefCallback(PreferenceState::C_WINDOW_HEIGHT, this, [&](const float &f) {resize(ivec2(m_renderSize.x, f)); });
		FrameBuffer::initialize();

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

void Reflection_FBO::bindForReading(const unsigned int & texture_unit)
{
	glBindTextureUnit(texture_unit, m_texture);
}

void Reflection_FBO::bindForWriting()
{
	FrameBuffer::bindForWriting();
	// Borrow the G_Buffer's depth-stencil texture
	glNamedFramebufferTexture(m_fbo, GL_DEPTH_STENCIL_ATTACHMENT, m_depth_stencil, 0);
}

void Reflection_FBO::resize(const vec2 & size)
{
	FrameBuffer::resize(size);

	glTextureImage2DEXT(m_texture, GL_TEXTURE_2D, 0, GL_RGB32F, m_renderSize.x, m_renderSize.y, 0, GL_RGB, GL_FLOAT, NULL);
	glNamedFramebufferTexture(m_fbo, GL_COLOR_ATTACHMENT0, m_texture, 0);
}

void Reflection_FBO::end()
{
	// return the G_Buffer's depth-stencil texture
	glNamedFramebufferTexture(m_fbo, GL_DEPTH_STENCIL_ATTACHMENT, 0, 0);
}
