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
	m_quadVAO = Asset_Primitive::Generate_VAO();
	m_shapeQuad->addCallback(this, [&]() { m_shapeQuad->updateVAO(m_quadVAO); });
	m_renderSize.x = m_enginePackage->addPrefCallback(PreferenceState::C_WINDOW_WIDTH, this, [&](const float &f) {resize(vec2(f, m_renderSize.y)); });
	m_renderSize.y = m_enginePackage->addPrefCallback(PreferenceState::C_WINDOW_HEIGHT, this, [&](const float &f) {resize(vec2(m_renderSize.x, f)); });

	glGenFramebuffers(1, &m_fbo);
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

	GLenum Status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (Status != GL_FRAMEBUFFER_COMPLETE && Status != GL_NO_ERROR) {
		std::string errorString = std::string(reinterpret_cast<char const *>(glewGetErrorString(Status)));
		MSG_Manager::Error(MSG_Manager::FBO_INCOMPLETE, "Lighting Buffer", errorString);
		return;
	}
}

void HDR_Tech::applyEffect()
{
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_STENCIL_TEST);
	glDepthMask(GL_FALSE);
	glDisable(GL_BLEND);
	//glDisable(GL_CULL_FACE);

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo);
	glClear(GL_COLOR_BUFFER_BIT);

	m_shaderHDR->bind();
	glBindVertexArray(m_quadVAO);
	glDrawArrays(GL_TRIANGLES, 0, m_shapeQuad->getSize());
	glBindVertexArray(0);
	Asset_Shader::Release();
}

void HDR_Tech::bindForReading()
{
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_texture);
}

void HDR_Tech::resize(const vec2 & size)
{
	m_renderSize = size;
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo);

	glBindTexture(GL_TEXTURE_2D, m_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, m_renderSize.x, m_renderSize.y, 0, GL_RGB, GL_FLOAT, NULL);

	// restore default FBO
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}
