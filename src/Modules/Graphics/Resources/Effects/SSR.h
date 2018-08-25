#pragma once
#ifndef SSR_H
#define SSR_H

#include "Modules\Graphics\Resources\Effects\Effect_Base.h"
#include "Assets\Asset_Shader.h"
#include "Assets\Asset_Primitive.h"
#include "Assets\Asset_Texture.h"
#include "Utilities\GL\StaticBuffer.h"
#include "Utilities\GL\FBO.h"
#include "Engine.h"


/** A post-processing technique for deriving extra reflection information from the viewport itself. */
class SSR : public Effect_Base {
public:
	// (de)Constructors
	/** Virtual Destructor. */
	~SSR() {
		m_engine->removePrefCallback(PreferenceState::C_WINDOW_WIDTH, this);
		m_engine->removePrefCallback(PreferenceState::C_WINDOW_HEIGHT, this);
		m_engine->removePrefCallback(PreferenceState::C_SSR, this);
		if (m_shapeQuad.get()) m_shapeQuad->removeCallback(this);
		glDeleteFramebuffers(1, &m_fboID);
		glDeleteTextures(1, &m_textureID);
		glDeleteTextures(1, &m_bayerID);
		glDeleteVertexArrays(1, &m_quadVAO);
	}
	/** Constructor. */
	SSR(Engine * engine, FBO_Base * geometryFBO, FBO_Base * lightingFBO, FBO_Base * reflectionFBO) {
		// Default Parameters
		m_engine = engine;
		m_geometryFBO = geometryFBO;
		m_lightingFBO = lightingFBO;
		m_reflectionFBO = reflectionFBO;

		// Asset Loading
		m_shaderSSR = Asset_Shader::Create(m_engine, "Effects\\SSR");
		m_shaderCopy = Asset_Shader::Create(m_engine, "Effects\\Copy Texture");
		m_shaderConvMips = Asset_Shader::Create(m_engine, "Effects\\Gaussian Blur MIP");
		m_brdfMap = Asset_Texture::Create(m_engine, "brdfLUT.png", GL_TEXTURE_2D, false, false);
		m_shapeQuad = Asset_Primitive::Create(m_engine, "quad");

		// Preference Callbacks
		m_renderSize.x = m_engine->addPrefCallback(PreferenceState::C_WINDOW_WIDTH, this, [&](const float &f) {resize(glm::ivec2(f, m_renderSize.y)); });
		m_renderSize.y = m_engine->addPrefCallback(PreferenceState::C_WINDOW_HEIGHT, this, [&](const float &f) {resize(glm::ivec2(m_renderSize.x, f)); });
		m_enabled = m_engine->addPrefCallback(PreferenceState::C_SSR, this, [&](const float &f) { m_enabled = (bool)f; });

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

		// GL loading
		m_fboID = 0;
		m_textureID = 0;
		glCreateFramebuffers(1, &m_fboID);
		glCreateTextures(GL_TEXTURE_2D, 1, &m_textureID);
		glTextureParameteri(m_textureID, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTextureParameteri(m_textureID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTextureParameteri(m_textureID, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTextureParameteri(m_textureID, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTextureParameteri(m_textureID, GL_TEXTURE_MIN_LOD, 0);
		glTextureParameteri(m_textureID, GL_TEXTURE_MAX_LOD, 5);
		glTextureParameteri(m_textureID, GL_TEXTURE_BASE_LEVEL, 0);
		glTextureParameteri(m_textureID, GL_TEXTURE_MAX_LEVEL, 5);
		for (int x = 0; x < 6; ++x) {
			const glm::ivec2 size(floor(m_renderSize.x / pow(2, x)), floor(m_renderSize.y / pow(2, x)));
			glTextureImage2DEXT(m_textureID, GL_TEXTURE_2D, x, GL_RGB16F, size.x, size.y, 0, GL_RGB, GL_FLOAT, NULL);
		}
		glNamedFramebufferTexture(m_fboID, GL_COLOR_ATTACHMENT0, m_textureID, 0);
		glNamedFramebufferDrawBuffer(m_fboID, GL_COLOR_ATTACHMENT0);

		// Bayer matrix
		GLubyte data[16] = { 0,8,2,10,12,4,14,6,3,11,1,9,15,7,13,5 };
		glCreateTextures(GL_TEXTURE_2D, 1, &m_bayerID);
		glTextureStorage2D(m_bayerID, 1, GL_R8, 4, 4);
		glTextureSubImage2D(m_bayerID, 0, 0, 0, 4, 4, GL_RED, GL_UNSIGNED_BYTE, &data);
		glTextureParameteri(m_bayerID, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTextureParameteri(m_bayerID, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTextureParameteri(m_bayerID, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTextureParameteri(m_bayerID, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

		// Error Reporting
		const GLenum Status = glCheckNamedFramebufferStatus(m_fboID, GL_FRAMEBUFFER);
		if (Status != GL_FRAMEBUFFER_COMPLETE && Status != GL_NO_ERROR)
			m_engine->reportError(MessageManager::FBO_INCOMPLETE, "SSR Framebuffer", std::string(reinterpret_cast<char const *>(glewGetErrorString(Status))));
		if (!glIsTexture(m_textureID))
			m_engine->reportError(MessageManager::TEXTURE_INCOMPLETE, "SSR Texture");
		if (!glIsTexture(m_bayerID))
			m_engine->reportError(MessageManager::TEXTURE_INCOMPLETE, "SSR - Bayer matrix texture");
	}


	// Interface Implementations.
	virtual void applyEffect(const float & deltaTime) {
		if (!m_shaderCopy->existsYet() || !m_shaderConvMips->existsYet() || !m_shaderSSR->existsYet() || !m_brdfMap->existsYet() || !m_quadVAOLoaded)
			return;
		glBindVertexArray(m_quadVAO);
		m_quadIndirectBuffer.bindBuffer(GL_DRAW_INDIRECT_BUFFER);

		updateMIPChain();

		glEnable(GL_BLEND);
		glBlendEquation(GL_FUNC_ADD);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		m_lightingFBO->bindForWriting();
		m_geometryFBO->bindForReading();
		glBindTextureUnit(4, m_textureID);
		m_brdfMap->bind(5);
		glBindTextureUnit(6, m_bayerID);
		m_shaderSSR->bind();
		glDrawArraysIndirect(GL_TRIANGLES, 0);
		glBlendFunc(GL_ONE, GL_ONE);
		glDisable(GL_BLEND);
	}


private:
	// Private Methods
	/** Convolute the lighting buffer into each of its mip levels. */
	void updateMIPChain() {
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_BLEND);

		// Copy lighting texture to one with a MIP chain
		m_shaderCopy->bind();
		m_lightingFBO->bindForReading();
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fboID);
		GLfloat clearColor[] = { 0.0f, 0.0f, 0.0f };
		glClearNamedFramebufferfv(m_fboID, GL_COLOR, 0, clearColor);
		glDrawArraysIndirect(GL_TRIANGLES, 0);		

		// Blur MIP chain, reading from 1 MIP level and writing into next
		m_shaderConvMips->bind();
		glBindTextureUnit(0, m_textureID);
		for (int horizontal = 0; horizontal < 2; ++horizontal) {
			m_shaderConvMips->setUniform(0, horizontal);
			glm::ivec2 read_size = m_renderSize;
			for (int x = 1; x < 6; ++x) {
				// Ensure we are reading from MIP level x - 1
				m_shaderConvMips->setUniform(1, read_size);
				glTextureParameteri(m_textureID, GL_TEXTURE_BASE_LEVEL, x - 1);
				glTextureParameteri(m_textureID, GL_TEXTURE_MAX_LEVEL, x - 1);
				// Ensure we are writing to MIP level x
				glm::ivec2 write_size(floor(m_renderSize.x / pow(2, x)), floor(m_renderSize.y / pow(2, x)));
				glNamedFramebufferTexture(m_fboID, GL_COLOR_ATTACHMENT0, m_textureID, x);

				glViewport(0, 0, max(1.0f, write_size.x), max(1.0f, write_size.y));
				glDrawArraysIndirect(GL_TRIANGLES, 0);
				read_size = write_size;
			}
			// Blend second pass
			glEnable(GL_BLEND);
			glBlendEquation(GL_FUNC_ADD);
			glBlendFunc(GL_ONE, GL_ONE);
		}

		// Restore to default
		glTextureParameteri(m_textureID, GL_TEXTURE_BASE_LEVEL, 0);
		glTextureParameteri(m_textureID, GL_TEXTURE_MAX_LEVEL, 5);
		glNamedFramebufferTexture(m_fboID, GL_COLOR_ATTACHMENT0, m_textureID, 0);
		glViewport(0, 0, m_renderSize.x, m_renderSize.y);
	}
	/** Resize the frame buffer.
	@param	size	the new size of the frame buffer */
	void resize(const glm::ivec2 & size) {
		m_renderSize = size;
		for (int x = 0; x < 6; ++x) {
			const glm::ivec2 size(floor(m_renderSize.x / pow(2, x)), floor(m_renderSize.y / pow(2, x)));
			glTextureImage2DEXT(m_textureID, GL_TEXTURE_2D, x, GL_RGB16F, size.x, size.y, 0, GL_RGB, GL_FLOAT, NULL);
		}
		glNamedFramebufferTexture(m_fboID, GL_COLOR_ATTACHMENT0, m_textureID, 0);
	}


	// Private Attributes
	Engine * m_engine;
	Shared_Asset_Shader m_shaderSSR, m_shaderCopy, m_shaderConvMips;
	Shared_Asset_Texture m_brdfMap;
	Shared_Asset_Primitive m_shapeQuad;
	glm::ivec2 m_renderSize;
	GLuint m_fboID, m_textureID;
	GLuint m_bayerID;
	GLuint m_quadVAO;
	bool m_quadVAOLoaded;
	StaticBuffer m_quadIndirectBuffer;
	FBO_Base * m_geometryFBO, * m_lightingFBO, *m_reflectionFBO;
};

#endif // SSR_H