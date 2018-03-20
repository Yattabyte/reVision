#include "Systems\Graphics\FX Techniques\Bloom_Tech.h"
#include "Systems\GraphiCS\Frame Buffers\Lighting_Buffer.h"
#include "Systems\Graphics\VisualFX.h"
#include "Utilities\EnginePackage.h"


Bloom_Tech::~Bloom_Tech()
{
	glDeleteTextures(2, m_texturesGB);
	glDeleteTextures(1, &m_texture);
	glDeleteFramebuffers(1, &m_fbo);
	m_enginePackage->removePrefCallback(PreferenceState::C_WINDOW_WIDTH, this);
	m_enginePackage->removePrefCallback(PreferenceState::C_WINDOW_HEIGHT, this);
	m_enginePackage->removePrefCallback(PreferenceState::C_BLOOM_STRENGTH, this);
	if (m_shapeQuad.get()) m_shapeQuad->removeCallback(this);
}

Bloom_Tech::Bloom_Tech(EnginePackage * enginePackage, Lighting_Buffer * lBuffer, VisualFX * visualFX)
{
	m_enginePackage = enginePackage;
	m_fbo = 0;
	m_texture = 0; 
	m_texturesGB[0] = 0;
	m_texturesGB[1] = 0;
	m_lBuffer = lBuffer;
	m_visualFX = visualFX;

	Asset_Loader::load_asset(m_shaderBloomExtract, "FX\\bloomExtraction");
	Asset_Loader::load_asset(m_shapeQuad, "quad");
	m_quadVAO = Asset_Primitive::Generate_VAO();
	m_shapeQuad->addCallback(this, [&]() { m_shapeQuad->updateVAO(m_quadVAO); });
	m_renderSize.x = m_enginePackage->addPrefCallback(PreferenceState::C_WINDOW_WIDTH, this, [&](const float &f) {resize(vec2(f, m_renderSize.y)); });
	m_renderSize.y = m_enginePackage->addPrefCallback(PreferenceState::C_WINDOW_HEIGHT, this, [&](const float &f) {resize(vec2(m_renderSize.x, f)); });
	m_bloomStrength = m_enginePackage->addPrefCallback(PreferenceState::C_BLOOM_STRENGTH, this, [&](const float &f) {setBloomStrength(f); });	

	glCreateFramebuffers(1, &m_fbo);
	glCreateTextures(GL_TEXTURE_2D, 1, &m_texture);
	glTextureImage2DEXT(m_texture, GL_TEXTURE_2D, 0, GL_RGB32F, m_renderSize.x, m_renderSize.y, 0, GL_RGB, GL_FLOAT, NULL);
	glTextureParameteri(m_texture, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTextureParameteri(m_texture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTextureParameteri(m_texture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri(m_texture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glNamedFramebufferTexture(m_fbo, GL_COLOR_ATTACHMENT0, m_texture, 0);
	glNamedFramebufferDrawBuffer(m_fbo, GL_COLOR_ATTACHMENT0);

	glCreateTextures(GL_TEXTURE_2D, 2, m_texturesGB);
	for (int x = 0; x < 2; ++x) {
		glTextureImage2DEXT(m_texturesGB[x], GL_TEXTURE_2D, 0, GL_RGB32F, m_renderSize.x, m_renderSize.y, 0, GL_RGB, GL_FLOAT, NULL);
		glTextureParameteri(m_texturesGB[x], GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureParameteri(m_texturesGB[x], GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTextureParameteri(m_texturesGB[x], GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTextureParameteri(m_texturesGB[x], GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	}
}

void Bloom_Tech::applyEffect()
{
	// Extract bright regions from lighting buffer
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
	m_shaderBloomExtract->bind();
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo);
	glBindVertexArray(m_quadVAO);
	m_lBuffer->bindForReading();
	glDrawArrays(GL_TRIANGLES, 0, m_shapeQuad->getSize());

	// Blur bright regions
	m_visualFX->applyGaussianBlur(m_texture, m_texturesGB, m_renderSize, m_bloomStrength);

	// Re-attach our bloom texture (was detached to allow for convolution)
	glNamedFramebufferTexture(m_fbo, GL_COLOR_ATTACHMENT0, m_texture, 0);
	glBindVertexArray(0);
	glEnable(GL_DEPTH_TEST);
	Asset_Shader::Release();
}

void Bloom_Tech::bindForReading()
{
	m_lBuffer->bindForReading();
	glBindTextureUnit(1, m_texture);
}

void Bloom_Tech::setBloomStrength(const int & strength)
{
	m_bloomStrength = strength;
}

void Bloom_Tech::resize(const vec2 & size)
{
	m_renderSize = size;
	glTextureImage2DEXT(m_texture, GL_TEXTURE_2D, 0, GL_RGB32F, m_renderSize.x, m_renderSize.y, 0, GL_RGB, GL_FLOAT, NULL);
	glNamedFramebufferTexture(m_fbo, GL_COLOR_ATTACHMENT0, m_texture, 0);
	for (int x = 0; x < 2; ++x) 
		glTextureImage2DEXT(m_texturesGB[x], GL_TEXTURE_2D, 0, GL_RGB32F, m_renderSize.x, m_renderSize.y, 0, GL_RGB, GL_FLOAT, NULL);
}
