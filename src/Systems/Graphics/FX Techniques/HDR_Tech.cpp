#include "Systems\Graphics\FX Techniques\HDR_Tech.h"
#include "Managers\Message_Manager.h"
#include "Utilities\EnginePackage.h"
#include <algorithm>


HDR_Tech::~HDR_Tech()
{
	// Destroy OpenGL objects
	glDeleteTextures(1, &m_texture);
	glDeleteFramebuffers(1, &m_fbo);
	m_enginePackage->removePrefCallback(PreferenceState::C_WINDOW_WIDTH, this);
	m_enginePackage->removePrefCallback(PreferenceState::C_WINDOW_HEIGHT, this);
	if (m_shapeQuad.get()) m_shapeQuad->removeCallback(this);
}

HDR_Tech::HDR_Tech(EnginePackage * enginePackage)
{
	m_fbo = 0;
	m_texture = 0;
	m_enginePackage = enginePackage;
	Asset_Loader::load_asset(m_shaderHDR, "FX\\HDR");
	Asset_Loader::load_asset(m_shapeQuad, "quad");
	m_vaoLoaded = false;
	m_quadVAO = Asset_Primitive::Generate_VAO();
	m_shapeQuad->addCallback(this, [&]() { m_shapeQuad->updateVAO(m_quadVAO); m_vaoLoaded = true; });
	m_renderSize.x = m_enginePackage->addPrefCallback(PreferenceState::C_WINDOW_WIDTH, this, [&](const float &f) {resize(vec2(f, m_renderSize.y)); });
	m_renderSize.y = m_enginePackage->addPrefCallback(PreferenceState::C_WINDOW_HEIGHT, this, [&](const float &f) {resize(vec2(m_renderSize.x, f)); });

	GLuint quadData[4] = { 6, 1, 0, 0 }; // count, primCount, first, reserved
	m_quadIndirectBuffer = StaticBuffer(sizeof(GLuint) * 4, quadData);

	glCreateFramebuffers(1, &m_fbo);
	glCreateTextures(GL_TEXTURE_2D, 1, &m_texture);
	glTextureImage2DEXT(m_texture, GL_TEXTURE_2D, 0, GL_RGB16F, m_renderSize.x, m_renderSize.y, 0, GL_RGB, GL_FLOAT, NULL);
	glTextureParameteri(m_texture, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTextureParameteri(m_texture, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTextureParameteri(m_texture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri(m_texture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glNamedFramebufferTexture(m_fbo, GL_COLOR_ATTACHMENT0, m_texture, 0);
	glNamedFramebufferDrawBuffer(m_fbo, GL_COLOR_ATTACHMENT0);

	GLenum Status = glCheckNamedFramebufferStatus(m_fbo, GL_FRAMEBUFFER);
	if (Status != GL_FRAMEBUFFER_COMPLETE && Status != GL_NO_ERROR)
		MSG_Manager::Error(MSG_Manager::FBO_INCOMPLETE, "Lighting Buffer", std::string(reinterpret_cast<char const *>(glewGetErrorString(Status))));
}

void HDR_Tech::applyEffect()
{
	if (m_shaderHDR->existsYet() && m_vaoLoaded) {
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_STENCIL_TEST);
		glDepthMask(GL_FALSE);
		glDisable(GL_BLEND);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo);
		glClear(GL_COLOR_BUFFER_BIT);

		m_shaderHDR->bind();
		glBindVertexArray(m_quadVAO);
		m_quadIndirectBuffer.bindBuffer(GL_DRAW_INDIRECT_BUFFER);
		glDrawArraysIndirect(GL_TRIANGLES, 0);
	}
}

void HDR_Tech::bindForReading()
{
	glBindTextureUnit(0, m_texture);
}

void HDR_Tech::resize(const vec2 & size)
{
	m_renderSize = size;

	glTextureImage2DEXT(m_texture, GL_TEXTURE_2D, 0, GL_RGB16F, m_renderSize.x, m_renderSize.y, 0, GL_RGB, GL_FLOAT, NULL);
	glNamedFramebufferTexture(m_fbo, GL_COLOR_ATTACHMENT0, m_texture, 0);
}
