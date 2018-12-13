#pragma once
#ifndef BLOOM_H
#define BLOOM_H

#include "Modules\Post Processing\Effects\GFX_PP_Effect.h"
#include "Assets\Asset_Shader.h"
#include "Assets\Asset_Primitive.h"
#include "Utilities\GL\StaticBuffer.h"
#include "Engine.h"


class Lighting_FBO;

/** A post-processing technique for generating bloom from a lighting buffer. */
class Bloom : public GFX_PP_Effect {
public:
	// (de)Constructors
	/** Virtual Destructor. */
	~Bloom() {
		// Update indicator
		m_aliveIndicator = false;

		// Destroy OpenGL objects
		glDeleteFramebuffers(1, &m_fboID);
		glDeleteTextures(1, &m_textureID);
		glDeleteTextures(2, m_textureIDS_GB); 
	}
	/** Constructor. */
	Bloom(Engine * engine, const GLuint & lightingFBOID, const GLuint & lightingTexID)
		: m_engine(engine), m_lightingFBOID(lightingFBOID), m_lightingTexID(lightingTexID) {
		// Asset Loading
		m_shaderBloomExtract = Asset_Shader::Create(m_engine, "Effects\\Bloom Extraction");
		m_shaderCopy = Asset_Shader::Create(m_engine, "Effects\\Copy Texture");
		m_shaderGB = Asset_Shader::Create(m_engine, "Effects\\Gaussian Blur");
		m_shapeQuad = Asset_Primitive::Create(engine, "quad");

		// Asset-Finished Callbacks
		m_shapeQuad->addCallback(m_aliveIndicator, [&]() mutable {
			const GLuint quadData[4] = { (GLuint)m_shapeQuad->getSize(), 1, 0, 0 }; // count, primCount, first, reserved
			m_quadIndirectBuffer = StaticBuffer(sizeof(GLuint) * 4, quadData, 0);
		});

		// Preference Callbacks
		auto & preferences = m_engine->getPreferenceState();
		preferences.getOrSetValue(PreferenceState::C_WINDOW_WIDTH, m_renderSize.x);
		preferences.addCallback(PreferenceState::C_WINDOW_WIDTH, m_aliveIndicator, [&](const float &f) { resize(glm::vec2(f, m_renderSize.y)); });
		preferences.getOrSetValue(PreferenceState::C_WINDOW_HEIGHT, m_renderSize.y);
		preferences.addCallback(PreferenceState::C_WINDOW_HEIGHT, m_aliveIndicator, [&](const float &f) { resize(glm::vec2(m_renderSize.x, f)); });
		preferences.getOrSetValue(PreferenceState::C_BLOOM, m_enabled);
		preferences.addCallback(PreferenceState::C_BLOOM, m_aliveIndicator, [&](const float &f) { m_enabled = (bool)f; });
		preferences.getOrSetValue(PreferenceState::C_BLOOM_STRENGTH, m_bloomStrength);
		preferences.addCallback(PreferenceState::C_BLOOM_STRENGTH, m_aliveIndicator, [&](const float &f) { setBloomStrength((int)f); });

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
		glCreateFramebuffers(1, &m_fbo_GB);

		// Error Reporting
		auto & msgMgr = m_engine->getMessageManager();
		if (glCheckNamedFramebufferStatus(m_fboID, GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			msgMgr.error("Bloom Framebuffer has encountered an error.");
		if (!glIsTexture(m_textureID))
			msgMgr.error("Bloom Texture is incomplete.");
		if (!glIsTexture(m_textureIDS_GB[0]))
			msgMgr.error("Bloom Gaussian blur texture #1 [0] is incomplete.");
		if (!glIsTexture(m_textureIDS_GB[1]))
			msgMgr.error("Bloom Gaussian blur texture #2 [1] is incomplete.");
	}


	// Interface Implementations.
	inline virtual void applyEffect(const float & deltaTime ) override {
		if (!m_shapeQuad->existsYet() || !m_shaderBloomExtract->existsYet() || !m_shaderCopy->existsYet() || !m_shaderGB->existsYet())
			return;
		// Extract bright regions from lighting buffer
		m_shaderBloomExtract->bind();
		glBindTextureUnit(0, m_lightingTexID);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fboID);
		glBindVertexArray(m_shapeQuad->m_vaoID);
		m_quadIndirectBuffer.bindBuffer(GL_DRAW_INDIRECT_BUFFER);
		glDrawArraysIndirect(GL_TRIANGLES, 0);

		// Blur bright regions
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo_GB);
		glNamedFramebufferTexture(m_fbo_GB, GL_COLOR_ATTACHMENT0, m_textureIDS_GB[0], 0);
		glNamedFramebufferTexture(m_fbo_GB, GL_COLOR_ATTACHMENT1, m_textureIDS_GB[1], 0);

		// Read from desired texture, blur into this frame buffer
		bool horizontal = false;
		glBindTextureUnit(0, m_textureID);
		glBindTextureUnit(1, m_textureIDS_GB[0]);
		glDrawBuffer(GL_COLOR_ATTACHMENT0 + horizontal);
		m_shaderGB->bind();
		m_shaderGB->setUniform(0, horizontal);
		m_shaderGB->setUniform(1, glm::vec2(m_renderSize));

		glBindVertexArray(m_shapeQuad->m_vaoID);
		m_quadIndirectBuffer.bindBuffer(GL_DRAW_INDIRECT_BUFFER);
		glDrawArraysIndirect(GL_TRIANGLES, 0);

		// Blur remainder of the times minus 1
		glBindTextureUnit(0, m_textureIDS_GB[1]);
		for (int i = 1; i < m_bloomStrength - 1; i++) {
			horizontal = !horizontal;
			glDrawBuffer(GL_COLOR_ATTACHMENT0 + horizontal);
			m_shaderGB->setUniform(0, horizontal);
			glDrawArraysIndirect(GL_TRIANGLES, 0);
		}

		// Last blur back into desired texture
		horizontal = !horizontal;
		glNamedFramebufferTexture(m_fbo_GB, GL_COLOR_ATTACHMENT0, m_textureID, 0);
		glDrawBuffer(GL_COLOR_ATTACHMENT0 + horizontal);
		m_shaderGB->setUniform(0, horizontal);
		glDrawArraysIndirect(GL_TRIANGLES, 0);

		// Re-attach our bloom texture (was detached to allow for convolution)
		glNamedFramebufferTexture(m_fboID, GL_COLOR_ATTACHMENT0, m_textureID, 0);

		// Copy to lighting buffer
		glEnable(GL_BLEND);
		glBlendEquation(GL_FUNC_ADD);
		glBlendFunc(GL_ONE, GL_ONE);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_lightingFBOID);
		glBindTextureUnit(0, m_textureID);
		m_shaderCopy->bind();
		glBindVertexArray(m_shapeQuad->m_vaoID);
		m_quadIndirectBuffer.bindBuffer(GL_DRAW_INDIRECT_BUFFER);
		glDrawArraysIndirect(GL_TRIANGLES, 0);
		glDisable(GL_BLEND);
		glBindTextureUnit(0, m_lightingTexID);
	}


private:
	// Private Methods
	/** Change the strength of the bloom effect.
	@param	strength		the new strength of the bloom effect */
	inline void setBloomStrength(const int &strength) {
		m_bloomStrength = strength;
	}
	/** Resize the frame buffer.
	@param	size	the new size of the frame buffer */
	inline void resize(const glm::vec2 & size) {
		m_renderSize = size;
		glTextureImage2DEXT(m_textureID, GL_TEXTURE_2D, 0, GL_RGB16F, m_renderSize.x, m_renderSize.y, 0, GL_RGB, GL_FLOAT, NULL);
		glNamedFramebufferTexture(m_fboID, GL_COLOR_ATTACHMENT0, m_textureID, 0);
		for (int x = 0; x < 2; ++x)
			glTextureImage2DEXT(m_textureIDS_GB[x], GL_TEXTURE_2D, 0, GL_RGB16F, m_renderSize.x, m_renderSize.y, 0, GL_RGB, GL_FLOAT, NULL);
	}


	// Private Attributes
	Engine * m_engine = nullptr;
	Shared_Asset_Shader m_shaderBloomExtract, m_shaderCopy, m_shaderGB;
	Shared_Asset_Primitive m_shapeQuad;
	StaticBuffer m_quadIndirectBuffer;
	GLuint m_lightingFBOID = 0, m_lightingTexID = 0, m_fboID = 0, m_textureID = 0, m_textureIDS_GB[2] = { 0,0 }, m_fbo_GB = 0;
	glm::ivec2 m_renderSize = glm::ivec2(1);
	int m_bloomStrength = 5;
	std::shared_ptr<bool> m_aliveIndicator = std::make_shared<bool>(true);
};

#endif // BLOOM_H