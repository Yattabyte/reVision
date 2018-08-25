#pragma once
#ifndef HDR_H
#define HDR_H

#include "Modules\Graphics\Resources\Effects\Effect_Base.h"
#include "Assets\Asset_Shader.h"
#include "Assets\Asset_Primitive.h"
#include "Utilities\GL\StaticBuffer.h"
#include "Utilities\GL\FBO.h"
#include "Engine.h"


/** A post-processing technique for joining together light + bloom, and applying hdr to it by tone-mapping it and gamma correcting it. */
class HDR : public Effect_Base {
public:
	// (de)Constructors
	/** Virtual Destructor. */
	~HDR() {
		// Destroy OpenGL objects
		glDeleteFramebuffers(1, &m_fboID);
		glDeleteTextures(1, &m_textureID);
		glDeleteVertexArrays(1, &m_quadVAO);
		m_engine->removePrefCallback(PreferenceState::C_WINDOW_WIDTH, this);
		m_engine->removePrefCallback(PreferenceState::C_WINDOW_HEIGHT, this);
		if (m_shapeQuad.get()) m_shapeQuad->removeCallback(this);
	}
	/** Constructor. */
	HDR(Engine * engine, FBO_Base * lightingFBO) {
		// Default Parameters
		m_engine = engine;
		m_lightingFBO = lightingFBO;

		// Asset Loading
		m_shaderHDR = Asset_Shader::Create(m_engine, "Effects\\HDR");
		m_shapeQuad = Asset_Primitive::Create(engine, "quad");

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
		m_renderSize.x = m_engine->addPrefCallback(PreferenceState::C_WINDOW_WIDTH, this, [&](const float &f) {resize(glm::ivec2(f, m_renderSize.y)); });
		m_renderSize.y = m_engine->addPrefCallback(PreferenceState::C_WINDOW_HEIGHT, this, [&](const float &f) {resize(glm::ivec2(m_renderSize.x, f)); });

		// GL loading
		m_fboID = 0;
		m_textureID = 0;
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
		if (Status != GL_FRAMEBUFFER_COMPLETE && Status != GL_NO_ERROR)
			m_engine->reportError(MessageManager::FBO_INCOMPLETE, "HDR Framebuffer", std::string(reinterpret_cast<char const *>(glewGetErrorString(Status))));
		if (!glIsTexture(m_textureID))
			m_engine->reportError(MessageManager::TEXTURE_INCOMPLETE, "HDR Texture");
	}


	// Interface Implementations.
	virtual void applyEffect(const float & deltaTime) {
		if (!m_shaderHDR->existsYet() || !m_quadVAOLoaded)
			return;
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fboID);
		glClear(GL_COLOR_BUFFER_BIT);

		m_shaderHDR->bind();
		m_shaderHDR->Set_Uniform(0, 1.0f);
		m_lightingFBO->bindForReading();
		glBindVertexArray(m_quadVAO);
		m_quadIndirectBuffer.bindBuffer(GL_DRAW_INDIRECT_BUFFER);
		glDrawArraysIndirect(GL_TRIANGLES, 0);	
		
		// Bind for reading by next effect	
		glBindTextureUnit(0, m_textureID);
	}


private:
	// Private Methods
	/** Resize the frame buffer.
	@param	size	the new size of the frame buffer */
	void resize(const glm::ivec2 & size) {
		m_renderSize = size;
		glTextureImage2DEXT(m_textureID, GL_TEXTURE_2D, 0, GL_RGB16F, m_renderSize.x, m_renderSize.y, 0, GL_RGB, GL_FLOAT, NULL);
		glNamedFramebufferTexture(m_fboID, GL_COLOR_ATTACHMENT0, m_textureID, 0);
	}


	// Private Attributes
	FBO_Base * m_lightingFBO;
	GLuint m_fboID, m_textureID;
	glm::ivec2 m_renderSize; 
	Engine * m_engine;
	Shared_Asset_Shader m_shaderHDR;
	Shared_Asset_Primitive m_shapeQuad;
	GLuint m_quadVAO;
	bool m_quadVAOLoaded;
	StaticBuffer m_quadIndirectBuffer;
};

#endif // HDR_H