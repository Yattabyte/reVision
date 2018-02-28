#include "Systems\Graphics\FX Techniques\Bloom_Tech.h"
#include "Systems\GraphiCS\Frame Buffers\Lighting_Buffer.h"
#include "Systems\Graphics\VisualFX.h"
#include "Utilities\EnginePackage.h"


struct Primitive_Observer : Asset_Observer {
	Primitive_Observer(Shared_Asset_Primitive & asset, const GLuint vao) : Asset_Observer(asset.get()), m_vao_id(vao) {};
	virtual void Notify_Finalized() {
		if (m_asset->existsYet())
			dynamic_pointer_cast<Asset_Primitive>(m_asset)->updateVAO(m_vao_id);
	}
	GLuint m_vao_id;
};

Bloom_Tech::~Bloom_Tech()
{
	glDeleteTextures(2, m_texturesGB);
	glDeleteTextures(1, &m_texture);
	glDeleteFramebuffers(1, &m_fbo);
	m_enginePackage->removePrefCallback(PreferenceState::C_WINDOW_WIDTH, this);
	m_enginePackage->removePrefCallback(PreferenceState::C_WINDOW_HEIGHT, this);
	m_enginePackage->removePrefCallback(PreferenceState::C_BLOOM_STRENGTH, this);
	delete m_QuadObserver;
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

	m_renderSize.x = m_enginePackage->addPrefCallback(PreferenceState::C_WINDOW_WIDTH, this, [&](const float &f) {resize(vec2(f, m_renderSize.y)); });
	m_renderSize.y = m_enginePackage->addPrefCallback(PreferenceState::C_WINDOW_HEIGHT, this, [&](const float &f) {resize(vec2(m_renderSize.x, f)); });
	m_bloomStrength = m_enginePackage->addPrefCallback(PreferenceState::C_BLOOM_STRENGTH, this, [&](const float &f) {setBloomStrength(f); });

	Asset_Loader::load_asset(m_shaderBloomExtract, "FX\\bloomExtraction");
	Asset_Loader::load_asset(m_shapeQuad, "quad");
	m_quadVAO = Asset_Primitive::Generate_VAO();
	m_QuadObserver = (void*)(new Primitive_Observer(m_shapeQuad, m_quadVAO));

	glGenFramebuffers(1, &m_fbo);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo);
	glGenTextures(1, &m_texture);
	glBindTexture(GL_TEXTURE_2D, m_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, m_renderSize.x, m_renderSize.y, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_texture, 0);
	glDrawBuffer(GL_COLOR_ATTACHMENT0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glGenTextures(2, m_texturesGB);
	for (int x = 0; x < 2; ++x) {
		glBindTexture(GL_TEXTURE_2D, m_texturesGB[x]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, m_renderSize.x, m_renderSize.y, 0, GL_RGB, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
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
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo);
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_texture, 0);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glBindVertexArray(0);
	glEnable(GL_DEPTH_TEST);
	Asset_Shader::Release();
}

void Bloom_Tech::bindForReading()
{
	m_lBuffer->bindForReading();
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, m_texture);
}

void Bloom_Tech::setBloomStrength(const int & strength)
{
	m_bloomStrength = strength;
}

void Bloom_Tech::resize(const vec2 & size)
{
	m_renderSize = size;
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo);
	glBindTexture(GL_TEXTURE_2D, m_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, m_renderSize.x, m_renderSize.y, 0, GL_RGB, GL_FLOAT, NULL);
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_texture, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	for (int x = 0; x < 2; ++x) {
		glBindTexture(GL_TEXTURE_2D, m_texturesGB[x]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, m_renderSize.x, m_renderSize.y, 0, GL_RGB, GL_FLOAT, NULL);		
	}
	glBindTexture(GL_TEXTURE_2D, 0);
}
