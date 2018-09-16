#pragma once
#ifndef BLOOM_H
#define BLOOM_H

#include "Modules\Graphics\Effects\Effect_Base.h"
#include "Modules\Graphics\Common\VisualFX.h"
#include "Assets\Asset_Shader.h"
#include "Assets\Asset_Primitive.h"
#include "Utilities\GL\StaticBuffer.h"
#include "Utilities\GL\FBO.h"
#include "Engine.h"


class Lighting_FBO;
class VisualFX;

/** A post processing technique for generating bloom from a lighting buffer. */
class Bloom : public Effect_Base {
public:
	// (de)Constructors
	/** Virtual Destructor. */
	~Bloom() {
		glDeleteFramebuffers(1, &m_fboID);
		glDeleteTextures(1, &m_textureID);
		glDeleteTextures(2, m_textureIDS_GB); 
		m_engine->removePrefCallback(PreferenceState::C_WINDOW_WIDTH, this);
		m_engine->removePrefCallback(PreferenceState::C_WINDOW_HEIGHT, this);
		m_engine->removePrefCallback(PreferenceState::C_BLOOM_STRENGTH, this);
		if (m_shapeQuad.get()) m_shapeQuad->removeCallback(this);
	}
	/** Constructor. */
	Bloom(Engine * engine, FBO_Base * lightingFBO, VisualFX * visualFX) 
		: m_engine(engine), m_lightingFBO(lightingFBO), m_visualFX(visualFX) {
		// Asset Loading
		m_shaderBloomExtract = Asset_Shader::Create(m_engine, "Effects\\Bloom Extraction");
		m_shaderCopy = Asset_Shader::Create(m_engine, "Effects\\Copy Texture");
		m_shapeQuad = Asset_Primitive::Create(engine, "quad");

		// Asset-Finished Callbacks
		m_shapeQuad->addCallback(this, [&]() mutable {
			const GLuint quadData[4] = { (GLuint)m_shapeQuad->getSize(), 1, 0, 0 }; // count, primCount, first, reserved
			m_quadIndirectBuffer = StaticBuffer(sizeof(GLuint) * 4, quadData, 0);
		});

		// Preference Callbacks
		m_renderSize.x = m_engine->addPrefCallback<int>(PreferenceState::C_WINDOW_WIDTH, this, [&](const float &f) {resize(glm::vec2(f, m_renderSize.y)); });
		m_renderSize.y = m_engine->addPrefCallback<int>(PreferenceState::C_WINDOW_HEIGHT, this, [&](const float &f) {resize(glm::vec2(m_renderSize.x, f)); });
		m_bloomStrength = m_engine->addPrefCallback<int>(PreferenceState::C_BLOOM_STRENGTH, this, [&](const float &f) {setBloomStrength((int)f); });
		m_enabled = m_engine->addPrefCallback<float>(PreferenceState::C_BLOOM, this, [&](const float &f) { m_enabled = (bool)f; });

		// GL Loading
		glCreateFramebuffers(1, &m_fboID);
		glCreateTextures(GL_TEXTURE_2D, 1, &m_textureID);
		glTextureImage2DEXT(m_textureID, GL_TEXTURE_2D, 0, GL_RGB16F, m_renderSize.x, m_renderSize.y, 0, GL_RGB, GL_FLOAT, NULL);
		glTextureParameteri(m_textureID, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTextureParameteri(m_textureID, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTextureParameteri(m_textureID, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTextureParameteri(m_textureID, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glNamedFramebufferTexture(m_fboID, GL_COLOR_ATTACHMENT0, m_textureID, 0);
		glNamedFramebufferDrawBuffer(m_fboID, GL_COLOR_ATTACHMENT0);

		glCreateTextures(GL_TEXTURE_2D, 2, m_textureIDS_GB);
		for (int x = 0; x < 2; ++x) {
			glTextureImage2DEXT(m_textureIDS_GB[x], GL_TEXTURE_2D, 0, GL_RGB16F, m_renderSize.x, m_renderSize.y, 0, GL_RGB, GL_FLOAT, NULL);
			glTextureParameteri(m_textureIDS_GB[x], GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTextureParameteri(m_textureIDS_GB[x], GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTextureParameteri(m_textureIDS_GB[x], GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTextureParameteri(m_textureIDS_GB[x], GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		}

		// Error Reporting
		const GLenum Status = glCheckNamedFramebufferStatus(m_fboID, GL_FRAMEBUFFER);
		if (Status != GL_FRAMEBUFFER_COMPLETE && Status != GL_NO_ERROR)
			m_engine->reportError(MessageManager::FBO_INCOMPLETE, "Bloom Framebuffer", std::string(reinterpret_cast<char const *>(glewGetErrorString(Status))));
		if (!glIsTexture(m_textureID))
			m_engine->reportError(MessageManager::TEXTURE_INCOMPLETE, "Bloom Texture");
		if (!glIsTexture(m_textureIDS_GB[0]))
			m_engine->reportError(MessageManager::TEXTURE_INCOMPLETE, "Bloom - Gaussian blur texture 0");
		if (!glIsTexture(m_textureIDS_GB[1]))
			m_engine->reportError(MessageManager::TEXTURE_INCOMPLETE, "Bloom - Gaussian blur texture 1");
	}


	// Interface Implementations.
	virtual void applyEffect(const float & deltaTime ) override {
		if (!m_shapeQuad->existsYet() || !m_shaderBloomExtract->existsYet() || !m_shaderCopy->existsYet())
			return;
		// Extract bright regions from lighting buffer
		m_shaderBloomExtract->bind();
		m_lightingFBO->bindForReading();
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fboID);
		glBindVertexArray(m_shapeQuad->m_vaoID);
		m_quadIndirectBuffer.bindBuffer(GL_DRAW_INDIRECT_BUFFER);
		glDrawArraysIndirect(GL_TRIANGLES, 0);

		// Blur bright regions
		m_visualFX->applyGaussianBlur(m_textureID, m_textureIDS_GB, m_renderSize, m_bloomStrength);

		// Re-attach our bloom texture (was detached to allow for convolution)
		glNamedFramebufferTexture(m_fboID, GL_COLOR_ATTACHMENT0, m_textureID, 0);

		// Copy to lighting buffer
		glEnable(GL_BLEND);
		glBlendEquation(GL_FUNC_ADD);
		glBlendFunc(GL_ONE, GL_ONE);
		m_lightingFBO->bindForWriting();
		glBindTextureUnit(0, m_textureID);
		m_shaderCopy->bind();
		glBindVertexArray(m_shapeQuad->m_vaoID);
		m_quadIndirectBuffer.bindBuffer(GL_DRAW_INDIRECT_BUFFER);
		glDrawArraysIndirect(GL_TRIANGLES, 0);
		glDisable(GL_BLEND);
	}


private:
	// Private Methods
	/** Change the strength of the bloom effect.
	@param	strength		the new strength of the bloom effect */
	void setBloomStrength(const int &strength) {
		m_bloomStrength = strength;
	}
	/** Resize the frame buffer.
	@param	size	the new size of the frame buffer */
	void resize(const glm::vec2 & size) {
		m_renderSize = size;
		glTextureImage2DEXT(m_textureID, GL_TEXTURE_2D, 0, GL_RGB16F, m_renderSize.x, m_renderSize.y, 0, GL_RGB, GL_FLOAT, NULL);
		glNamedFramebufferTexture(m_fboID, GL_COLOR_ATTACHMENT0, m_textureID, 0);
		for (int x = 0; x < 2; ++x)
			glTextureImage2DEXT(m_textureIDS_GB[x], GL_TEXTURE_2D, 0, GL_RGB16F, m_renderSize.x, m_renderSize.y, 0, GL_RGB, GL_FLOAT, NULL);
	}


	// Private Attributes
	Engine * m_engine = nullptr;
	FBO_Base * m_lightingFBO = nullptr;
	VisualFX * m_visualFX = nullptr;
	Shared_Asset_Shader m_shaderBloomExtract, m_shaderCopy;
	Shared_Asset_Primitive m_shapeQuad;
	StaticBuffer m_quadIndirectBuffer;
	GLuint m_fboID = 0, m_textureID = 0, m_textureIDS_GB[2] = { 0,0 };
	glm::ivec2 m_renderSize = glm::ivec2(1);
	int m_bloomStrength = 5;
};

#endif // BLOOM_H