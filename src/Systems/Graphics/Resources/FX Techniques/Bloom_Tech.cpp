#include "Systems\Graphics\Resources\FX Techniques\Bloom_Tech.h"
#include "Systems\Graphics\Resources\Frame Buffers\Lighting_FBO.h"
#include "Systems\Graphics\Resources\VisualFX.h"
#include "Engine.h"


Bloom_Tech::~Bloom_Tech()
{
	glDeleteTextures(2, m_texturesGB);
	glDeleteTextures(1, &m_texture);
	glDeleteFramebuffers(1, &m_fbo);
	m_engine->removePrefCallback(PreferenceState::C_WINDOW_WIDTH, this);
	m_engine->removePrefCallback(PreferenceState::C_WINDOW_HEIGHT, this);
	m_engine->removePrefCallback(PreferenceState::C_BLOOM_STRENGTH, this);
	if (m_shapeQuad.get()) m_shapeQuad->removeCallback(this);
}

Bloom_Tech::Bloom_Tech(Engine * engine, Lighting_FBO * lightingFBO, VisualFX * visualFX)
{
	// Default Parameters
	m_engine = engine;
	m_fbo = 0;
	m_texture = 0; 
	m_texturesGB[0] = 0;
	m_texturesGB[1] = 0;
	m_lightingFBO = lightingFBO;
	m_visualFX = visualFX;

	// Asset Loading
	m_engine->createAsset(m_shaderBloomExtract, std::string("FX\\bloomExtraction"), true);
	m_engine->createAsset(m_shapeQuad, std::string("quad"), true);

	// Primitive Construction
	m_quadVAOLoaded = false;
	m_quadVAO = Asset_Primitive::Generate_VAO();
	m_quadIndirectBuffer = StaticBuffer(sizeof(GLuint) * 4, 0);
	m_shapeQuad->addCallback(this, [&]() mutable { 
		m_quadVAOLoaded = true;
		m_shapeQuad->updateVAO(m_quadVAO); 
		const GLuint quadData[4] = { m_shapeQuad->getSize(), 1, 0, 0 }; // count, primCount, first, reserved
		m_quadIndirectBuffer.write(0, sizeof(GLuint) * 4, quadData);
	});

	// Preference Callbacks
	m_renderSize.x = m_engine->addPrefCallback(PreferenceState::C_WINDOW_WIDTH, this, [&](const float &f) {resize(glm::vec2(f, m_renderSize.y)); });
	m_renderSize.y = m_engine->addPrefCallback(PreferenceState::C_WINDOW_HEIGHT, this, [&](const float &f) {resize(glm::vec2(m_renderSize.x, f)); });
	m_bloomStrength = m_engine->addPrefCallback(PreferenceState::C_BLOOM_STRENGTH, this, [&](const float &f) {setBloomStrength(f); });	
	
	// GL Loading
	glCreateFramebuffers(1, &m_fbo);
	glCreateTextures(GL_TEXTURE_2D, 1, &m_texture);
	glTextureImage2DEXT(m_texture, GL_TEXTURE_2D, 0, GL_RGB16F, m_renderSize.x, m_renderSize.y, 0, GL_RGB, GL_FLOAT, NULL);
	glTextureParameteri(m_texture, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTextureParameteri(m_texture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTextureParameteri(m_texture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri(m_texture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glNamedFramebufferTexture(m_fbo, GL_COLOR_ATTACHMENT0, m_texture, 0);
	glNamedFramebufferDrawBuffer(m_fbo, GL_COLOR_ATTACHMENT0);

	glCreateTextures(GL_TEXTURE_2D, 2, m_texturesGB);
	for (int x = 0; x < 2; ++x) {
		glTextureImage2DEXT(m_texturesGB[x], GL_TEXTURE_2D, 0, GL_RGB16F, m_renderSize.x, m_renderSize.y, 0, GL_RGB, GL_FLOAT, NULL);
		glTextureParameteri(m_texturesGB[x], GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureParameteri(m_texturesGB[x], GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTextureParameteri(m_texturesGB[x], GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTextureParameteri(m_texturesGB[x], GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	}

	// Error Reporting
	const GLenum Status = glCheckNamedFramebufferStatus(m_fbo, GL_FRAMEBUFFER);
	if (Status != GL_FRAMEBUFFER_COMPLETE && Status != GL_NO_ERROR)
		m_engine->reportError(MessageManager::FBO_INCOMPLETE, "Lighting Buffer", std::string(reinterpret_cast<char const *>(glewGetErrorString(Status))));
}

void Bloom_Tech::applyEffect()
{
	if (m_shaderBloomExtract->existsYet() && m_quadVAOLoaded) {
		// Extract bright regions from lighting buffer
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_BLEND);
		m_shaderBloomExtract->bind();
		m_lightingFBO->bindForReading();
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo);
		glBindVertexArray(m_quadVAO);
		m_quadIndirectBuffer.bindBuffer(GL_DRAW_INDIRECT_BUFFER);
		glDrawArraysIndirect(GL_TRIANGLES, 0);		

		// Blur bright regions
		m_visualFX->applyGaussianBlur(m_texture, m_texturesGB, m_renderSize, m_bloomStrength);

		// Re-attach our bloom texture (was detached to allow for convolution)
		glNamedFramebufferTexture(m_fbo, GL_COLOR_ATTACHMENT0, m_texture, 0);
		glEnable(GL_DEPTH_TEST);
	}
}

void Bloom_Tech::bindForReading()
{
	m_lightingFBO->bindForReading();
	glBindTextureUnit(1, m_texture);
}

void Bloom_Tech::setBloomStrength(const int & strength)
{
	m_bloomStrength = strength;
}

void Bloom_Tech::resize(const glm::vec2 & size)
{
	m_renderSize = size;
	glTextureImage2DEXT(m_texture, GL_TEXTURE_2D, 0, GL_RGB16F, m_renderSize.x, m_renderSize.y, 0, GL_RGB, GL_FLOAT, NULL);
	glNamedFramebufferTexture(m_fbo, GL_COLOR_ATTACHMENT0, m_texture, 0);
	for (int x = 0; x < 2; ++x) 
		glTextureImage2DEXT(m_texturesGB[x], GL_TEXTURE_2D, 0, GL_RGB16F, m_renderSize.x, m_renderSize.y, 0, GL_RGB, GL_FLOAT, NULL);
}