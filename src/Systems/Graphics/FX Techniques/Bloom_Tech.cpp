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

class Bloom_StrengthChangeCallback : public Callback_Container {
public:
	~Bloom_StrengthChangeCallback() {};
	Bloom_StrengthChangeCallback(Bloom_Tech * lBuffer) : m_LBuffer(lBuffer) {}
	void Callback(const float & value) {
		m_LBuffer->setBloomStrength((int)value);
	}
private:
	Bloom_Tech *m_LBuffer;
};
class Cam_WidthChangeCallback : public Callback_Container {
public:
	~Cam_WidthChangeCallback() {};
	Cam_WidthChangeCallback(Bloom_Tech * graphics) : m_Graphics(graphics) {}
	void Callback(const float & value) {
		m_Graphics->resize(vec2(value, m_preferenceState->getPreference(PreferenceState::C_WINDOW_HEIGHT)));
	}
private:
	Bloom_Tech *m_Graphics;
};
class Cam_HeightChangeCallback : public Callback_Container {
public:
	~Cam_HeightChangeCallback() {};
	Cam_HeightChangeCallback(Bloom_Tech * lBuffer) : m_Graphics(lBuffer) {}
	void Callback(const float & value) {
		m_Graphics->resize(vec2(m_preferenceState->getPreference(PreferenceState::C_WINDOW_WIDTH), value));
	}
private:
	Bloom_Tech *m_Graphics;
};

Bloom_Tech::~Bloom_Tech()
{
	glDeleteTextures(1, &m_texture);
	glDeleteTextures(2, m_texturesGB);
	glDeleteFramebuffers(1, &m_fbo);
	m_enginePackage->removeCallback(PreferenceState::C_WINDOW_HEIGHT, m_bloomStrengthChangeCallback);
	delete m_bloomStrengthChangeCallback;
	delete m_widthChangeCallback;
	delete m_heightChangeCallback;
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
	m_renderSize = vec2(m_enginePackage->getPreference(PreferenceState::C_WINDOW_WIDTH), m_enginePackage->getPreference(PreferenceState::C_WINDOW_HEIGHT));
	m_widthChangeCallback = new Cam_WidthChangeCallback(this);
	m_heightChangeCallback = new Cam_HeightChangeCallback(this);
	m_enginePackage->addCallback(PreferenceState::C_WINDOW_WIDTH, m_widthChangeCallback);
	m_enginePackage->addCallback(PreferenceState::C_WINDOW_HEIGHT, m_heightChangeCallback);
	m_bloomStrength = m_enginePackage->getPreference(PreferenceState::C_BLOOM_STRENGTH);

	m_bloomStrengthChangeCallback = new Bloom_StrengthChangeCallback(this);
	m_enginePackage->addCallback(PreferenceState::C_BLOOM_STRENGTH, m_bloomStrengthChangeCallback);
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
