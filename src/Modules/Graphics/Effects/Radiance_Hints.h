#pragma once
#ifndef RADIANCE_HINTS_H
#define RADIANCE_HINTS_H

#include "Modules\Graphics\Effects\Effect_Base.h"
#include "Modules\Graphics\Common\RH_Volume.h"
#include "Modules\Graphics\Common\FBO_LightBounce.h"
#include "Assets\Asset_Shader.h"
#include "Assets\Asset_Texture.h"
#include "Assets\Asset_Primitive.h"
#include "Utilities\GL\StaticBuffer.h"
#include "Utilities\GL\FBO.h"
#include "Engine.h"


/** A post-processing technique for approximating indirect diffuse lighting (irradiant light, global illumination, etc) */
class Radiance_Hints : public Effect_Base {
public:
	// (de)Constructors
	/** Virtual Destructor. */
	~Radiance_Hints() {
		// Update indicator
		m_aliveIndicator = false;

		// Destroy OpenGL objects
		glDeleteFramebuffers(1, &m_fboID);
		glDeleteTextures(1, &m_textureID);
	}
	/** Constructor. */
	Radiance_Hints(Engine * engine, FBO_Base * geometryFBO, FBO_Base * bounceFBO, std::shared_ptr<RH_Volume> volumeRH)
	: m_engine(engine), m_geometryFBO(geometryFBO), m_bounceFBO(bounceFBO), m_volumeRH(volumeRH) {
		// Asset Loading
		m_shaderRecon = Asset_Shader::Create(m_engine, "Effects\\RH Reconstruction");
		m_shaderRebounce = Asset_Shader::Create(m_engine, "Effects\\RH Rebounce");
		m_shapeQuad = Asset_Primitive::Create(m_engine, "quad");

		// Preference Callbacks
		m_renderSize.x = m_engine->addPrefCallback<int>(PreferenceState::C_WINDOW_WIDTH, m_aliveIndicator, [&](const float &f) {resize(glm::vec2(f, m_renderSize.y)); });
		m_renderSize.y = m_engine->addPrefCallback<int>(PreferenceState::C_WINDOW_HEIGHT, m_aliveIndicator, [&](const float &f) {resize(glm::vec2(m_renderSize.x, f)); });
		const GLuint m_bounceSize = m_engine->addPrefCallback<GLuint>(PreferenceState::C_RH_BOUNCE_SIZE, m_aliveIndicator, [&](const float &f) { m_rebounceFBO.resize((GLuint)f); });
		m_rebounceFBO.resize(m_bounceSize);

		// Asset-Finished callbacks
		m_shapeQuad->addCallback(m_aliveIndicator, [&, m_bounceSize]() mutable {
			// count, primCount, first, reserved
			const GLuint quadData[4] = { (GLuint)m_shapeQuad->getSize(), m_bounceSize, 0, 0 };
			m_quadIndirectBuffer = StaticBuffer(sizeof(GLuint) * 4, quadData, 0);
		});

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

		// Error Reporting
		const GLenum Status = glCheckNamedFramebufferStatus(m_fboID, GL_FRAMEBUFFER);
		if (Status != GL_FRAMEBUFFER_COMPLETE && Status != GL_NO_ERROR)
			m_engine->reportError(MessageManager::FBO_INCOMPLETE, "Radiance Hints Framebuffer", std::string(reinterpret_cast<char const *>(glewGetErrorString(Status))));
		if (!glIsTexture(m_textureID))
			m_engine->reportError(MessageManager::TEXTURE_INCOMPLETE, "Radiance Hints Texture");
	}


	// Interface Implementations.
	virtual void applyEffect(const float & deltaTime) override {
		if (!m_shapeQuad->existsYet() || !m_shaderRecon->existsYet() || !m_shaderRebounce->existsYet())
			return;
		
		// Clear buffers and bind common data
		m_rebounceFBO.clear();
		GLfloat clearColor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
		glClearNamedFramebufferfv(m_fboID, GL_COLOR, 0, clearColor);
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_BLEND);
		m_shaderRebounce->setUniform(1, m_volumeRH->m_max);
		m_shaderRebounce->setUniform(2, m_volumeRH->m_min);
		m_shaderRebounce->setUniform(4, m_volumeRH->m_resolution);
		m_shaderRebounce->setUniform(5, m_volumeRH->m_unitSize);
		m_shaderRecon->setUniform(1, m_volumeRH->m_max);
		m_shaderRecon->setUniform(2, m_volumeRH->m_min);
		m_shaderRecon->setUniform(3, m_volumeRH->m_resolution);
		glBindVertexArray(m_shapeQuad->m_vaoID);
		m_quadIndirectBuffer.bindBuffer(GL_DRAW_INDIRECT_BUFFER);

		// Bounce light a second time
		m_shaderRebounce->bind();
		m_bounceFBO->bindForReading(0);
		m_rebounceFBO.bindForWriting();
		glViewport(0, 0, (GLsizei)m_volumeRH->m_resolution, (GLsizei)m_volumeRH->m_resolution);
		glDrawArraysIndirect(GL_TRIANGLES, 0);
		glViewport(0, 0, m_renderSize.x, m_renderSize.y);

		// Reconstruct indirect radiance
		m_shaderRecon->bind();
		m_geometryFBO->bindForReading(0);
		m_rebounceFBO.bindForReading(4);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fboID);
		glDrawArraysIndirect(GL_TRIANGLES, 0);		

		// Bind for reading by next effect	
		glBindTextureUnit(4, m_textureID);
	}


private:
	// Private Methods
	/** Resize the frame buffer.
	@param	size	the new size of the frame buffer */
	void resize(const glm::vec2 & size) {
		m_renderSize = size;
		glTextureImage2DEXT(m_textureID, GL_TEXTURE_2D, 0, GL_RGB16F, m_renderSize.x, m_renderSize.y, 0, GL_RGB, GL_FLOAT, NULL);
		glNamedFramebufferTexture(m_fboID, GL_COLOR_ATTACHMENT0, m_textureID, 0);
	}


	// Private Attributes
	Engine * m_engine = nullptr;
	FBO_Base * m_geometryFBO = nullptr, * m_bounceFBO = nullptr;
	FBO_LightBounce	m_rebounceFBO;
	Shared_Asset_Shader m_shaderRecon, m_shaderRebounce;
	Shared_Asset_Primitive m_shapeQuad;
	StaticBuffer m_quadIndirectBuffer;
	std::shared_ptr<RH_Volume> m_volumeRH;
	glm::ivec2 m_renderSize = glm::ivec2(1);
	GLuint m_fboID = 0, m_textureID = 0;
	std::shared_ptr<bool> m_aliveIndicator = std::make_shared<bool>(true);
};

#endif // RADIANCE_HINTS_H