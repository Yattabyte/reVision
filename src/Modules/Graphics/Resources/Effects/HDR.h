#pragma once
#ifndef HDR_H
#define HDR_H

#include "Modules\Graphics\Resources\Effects\Effect_Base.h"
#include "Assets\Asset_Shader.h"
#include "Assets\Asset_Primitive.h"
#include "Utilities\GL\StaticBuffer.h"
#include "Utilities\GL\FBO.h"
#include "Engine.h"


/** A post-processing technique for tone-mapping and gamma correcting the final lighting product. */
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
	HDR(Engine * engine, FBO_Base * lightingFBO)
	: m_engine(engine), m_lightingFBO(lightingFBO) {
		// Asset Loading
		m_shaderHDR = Asset_Shader::Create(m_engine, "Effects\\HDR");
		m_shapeQuad = Asset_Primitive::Create(engine, "quad");

		// Primitive Construction
		m_quadVAO = Asset_Primitive::Generate_VAO();
		m_quadIndirectBuffer = StaticBuffer(sizeof(GLuint) * 4);
		m_shapeQuad->addCallback(this, [&]() mutable {
			m_quadVAOLoaded = true;
			m_shapeQuad->updateVAO(m_quadVAO);
			const GLuint quadData[4] = { (GLuint)m_shapeQuad->getSize(), 1, 0, 0 }; // count, primCount, first, reserved
			m_quadIndirectBuffer.write(0, sizeof(GLuint) * 4, quadData);
		});

		// Preference Callbacks
		m_renderSize.x = m_engine->addPrefCallback<int>(PreferenceState::C_WINDOW_WIDTH, this, [&](const float &f) {resize(glm::ivec2(f, m_renderSize.y)); });
		m_renderSize.y = m_engine->addPrefCallback<int>(PreferenceState::C_WINDOW_HEIGHT, this, [&](const float &f) {resize(glm::ivec2(m_renderSize.x, f)); });

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
		if (Status != GL_FRAMEBUFFER_COMPLETE && Status != GL_NO_ERROR)
			m_engine->reportError(MessageManager::FBO_INCOMPLETE, "HDR Framebuffer", std::string(reinterpret_cast<char const *>(glewGetErrorString(Status))));
		if (!glIsTexture(m_textureID))
			m_engine->reportError(MessageManager::TEXTURE_INCOMPLETE, "HDR Texture");
	}


	// Interface Implementations.
	virtual void applyEffect(const float & deltaTime) override {
		if (!m_shaderHDR->existsYet() || !m_quadVAOLoaded)
			return;
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fboID);
		GLfloat clearColor[] = { 0.0f, 0.0f, 0.0f };
		glClearNamedFramebufferfv(m_fboID, GL_COLOR, 0, clearColor);

		m_shaderHDR->bind();
		m_shaderHDR->setUniform(0, 1.0f);
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
	Engine * m_engine = nullptr;
	FBO_Base * m_lightingFBO = nullptr;
	GLuint m_fboID = 0, m_textureID = 0;
	glm::ivec2 m_renderSize = glm::ivec2(1);
	Shared_Asset_Shader m_shaderHDR;
	Shared_Asset_Primitive m_shapeQuad;
	GLuint m_quadVAO = 0;
	bool m_quadVAOLoaded = false;
	StaticBuffer m_quadIndirectBuffer;
};

#endif // HDR_H