#pragma once
#ifndef FXAA_H
#define FXAA_H

#include "Modules\Graphics\Effects\Effect_Base.h"
#include "Assets\Asset_Shader.h"
#include "Assets\Asset_Primitive.h"
#include "Utilities\GL\StaticBuffer.h"
#include "Engine.h"


/** A post-processing technique for applying fxaa to the currently bound 2D image. */
class FXAA : public Effect_Base {
public:
	// (de)Constructors
	/** Virtual Destructor. */
	~FXAA() {
		// Update indicator
		m_aliveIndicator = false;

		// Destroy OpenGL objects
		glDeleteFramebuffers(1, &m_fboID);
		glDeleteTextures(1, &m_textureID);
	}
	/** Constructor. */
	FXAA(Engine * engine) 
	: m_engine(engine) {
		// Asset Loading
		m_shaderFXAA = Asset_Shader::Create(m_engine, "Effects\\FXAA");
		m_shapeQuad = Asset_Primitive::Create(m_engine, "quad");

		// Asset-Finished Callbacks
		m_shapeQuad->addCallback(m_aliveIndicator, [&]() mutable {
			const GLuint quadData[4] = { (GLuint)m_shapeQuad->getSize(), 1, 0, 0 }; // count, primCount, first, reserved
			m_quadIndirectBuffer = StaticBuffer(sizeof(GLuint) * 4, quadData, 0);
		});

		// Preferences
		auto & preferences = m_engine->getPreferenceState();
		preferences.getOrSetValue(PreferenceState::C_WINDOW_WIDTH, m_renderSize.x);
		preferences.addCallback(PreferenceState::C_WINDOW_WIDTH, m_aliveIndicator, [&](const float &f) {resize(glm::ivec2(f, m_renderSize.y)); });
		preferences.getOrSetValue(PreferenceState::C_WINDOW_HEIGHT, m_renderSize.y);
		preferences.addCallback(PreferenceState::C_WINDOW_HEIGHT, m_aliveIndicator, [&](const float &f) {resize(glm::ivec2(m_renderSize.x, f)); });
		preferences.getOrSetValue(PreferenceState::C_FXAA, m_enabled);
		preferences.addCallback(PreferenceState::C_FXAA, m_aliveIndicator, [&](const float &f) { m_enabled = (bool)f; });

		// GL loading
		glCreateFramebuffers(1, &m_fboID);
		glCreateTextures(GL_TEXTURE_2D, 1, &m_textureID);
		glTextureImage2DEXT(m_textureID, GL_TEXTURE_2D, 0, GL_RGB16F, m_renderSize.x, m_renderSize.y, 0, GL_RGB, GL_FLOAT, NULL);
		glTextureParameteri(m_textureID, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTextureParameteri(m_textureID, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTextureParameteri(m_textureID, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTextureParameteri(m_textureID, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glNamedFramebufferTexture(m_fboID, GL_COLOR_ATTACHMENT0, m_textureID, 0);
		glNamedFramebufferDrawBuffer(m_fboID, GL_COLOR_ATTACHMENT0);
		
		// Error Reporting
		const GLenum Status = glCheckNamedFramebufferStatus(m_fboID, GL_FRAMEBUFFER);
		//if (Status != GL_FRAMEBUFFER_COMPLETE && Status != GL_NO_ERROR)
		//	m_engine->getMessageManager().error("FXAA Framebuffer is incomplete. Reason: \n" + std::string(reinterpret_cast<char const *>(glewGetErrorString(Status))));
		if (!glIsTexture(m_textureID))
			m_engine->getMessageManager().error("FXAA Texture is incomplete.");
	}


	// Interface Implementations.
	inline virtual void applyEffect(const float & deltaTime) override {
		if (!m_shapeQuad->existsYet() || !m_shaderFXAA->existsYet())
			return;
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fboID);
		m_shaderFXAA->bind();
		glBindVertexArray(m_shapeQuad->m_vaoID);
		m_quadIndirectBuffer.bindBuffer(GL_DRAW_INDIRECT_BUFFER);
		glDrawArraysIndirect(GL_TRIANGLES, 0);

		// Bind for reading by next effect	
		glBindTextureUnit(0, m_textureID);
	}
	

private:
	// Private Methods
	/** Resize the frame buffer.
	@param	size	the new size of the frame buffer */
	inline void resize(const glm::ivec2 & size) {
		m_renderSize = size;
		glTextureImage2DEXT(m_textureID, GL_TEXTURE_2D, 0, GL_RGB16F, m_renderSize.x, m_renderSize.y, 0, GL_RGB, GL_FLOAT, NULL);
		glNamedFramebufferTexture(m_fboID, GL_COLOR_ATTACHMENT0, m_textureID, 0);
	}


	// Private Attributes
	Engine * m_engine = nullptr;
	Shared_Asset_Shader m_shaderFXAA;
	Shared_Asset_Primitive m_shapeQuad;
	GLuint m_fboID = 0, m_textureID = 0;
	glm::ivec2 m_renderSize = glm::ivec2(1);
	StaticBuffer m_quadIndirectBuffer;
	std::shared_ptr<bool> m_aliveIndicator = std::make_shared<bool>(true);
};

#endif // FXAA_H