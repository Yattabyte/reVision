#pragma once
#ifndef SSR_H
#define SSR_H

#include "Modules/Graphics/Graphics_Technique.h"
#include "Assets/Shader.h"
#include "Assets/Primitive.h"
#include "Assets/Texture.h"
#include "Utilities/GL/StaticBuffer.h"
#include "Utilities/GL/FBO.h"
#include "Engine.h"


/** A core-rendering technique for deriving extra reflection information from the viewport itself. */
class SSR : public Graphics_Technique {
public:
	// Public (de)Constructors
	/** Virtual Destructor. */
	inline ~SSR() {
		// Update indicator
		m_aliveIndicator = false;

		// Destroy OpenGL objects
		glDeleteFramebuffers(1, &m_fboMipsID);
		glDeleteTextures(1, &m_textureMipsID);
		glDeleteTextures(1, &m_bayerID);
	}
	/** Constructor. */
	inline SSR(Engine * engine, FBO_Base * geometryFBO, FBO_Base * lightingFBO, FBO_Base * reflectionFBO)
		: m_engine(engine), m_geometryFBO(geometryFBO), m_lightingFBO(lightingFBO), m_reflectionFBO(reflectionFBO) {
		// Asset Loading
		m_shaderSSR1 = Shared_Shader(m_engine, "Effects\\SSR part 1");
		m_shaderSSR2 = Shared_Shader(m_engine, "Effects\\SSR part 2");
		m_shaderCopy = Shared_Shader(m_engine, "Effects\\Copy Texture");
		m_shaderConvMips = Shared_Shader(m_engine, "Effects\\Gaussian Blur MIP");
		m_shapeQuad = Shared_Primitive(m_engine, "quad");

		// Preferences
		auto & preferences = m_engine->getPreferenceState();
		preferences.getOrSetValue(PreferenceState::C_WINDOW_WIDTH, m_renderSize.x);
		preferences.addCallback(PreferenceState::C_WINDOW_WIDTH, m_aliveIndicator, [&](const float &f) { resize(glm::ivec2(f, m_renderSize.y)); });
		preferences.getOrSetValue(PreferenceState::C_WINDOW_HEIGHT, m_renderSize.y);
		preferences.addCallback(PreferenceState::C_WINDOW_HEIGHT, m_aliveIndicator, [&](const float &f) { resize(glm::ivec2(m_renderSize.x, f)); });
		preferences.getOrSetValue(PreferenceState::C_SSR, m_enabled);
		preferences.addCallback(PreferenceState::C_SSR, m_aliveIndicator, [&](const float &f) { m_enabled = (bool)f; });

		// Asset-Finished Callbacks
		m_shapeQuad->addCallback(m_aliveIndicator, [&]() mutable {
			const GLuint quadData[4] = { (GLuint)m_shapeQuad->getSize(), 1, 0, 0 }; // count, primCount, first, reserved
			m_quadIndirectBuffer = StaticBuffer(sizeof(GLuint) * 4, quadData, 0);
		});

		// GL loading
		glCreateFramebuffers(1, &m_fboMipsID);
		glCreateTextures(GL_TEXTURE_2D, 1, &m_textureMipsID);
		glTextureParameteri(m_textureMipsID, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTextureParameteri(m_textureMipsID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTextureParameteri(m_textureMipsID, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTextureParameteri(m_textureMipsID, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTextureParameteri(m_textureMipsID, GL_TEXTURE_MIN_LOD, 0);
		glTextureParameteri(m_textureMipsID, GL_TEXTURE_MAX_LOD, 5);
		glTextureParameteri(m_textureMipsID, GL_TEXTURE_BASE_LEVEL, 0);
		glTextureParameteri(m_textureMipsID, GL_TEXTURE_MAX_LEVEL, 5);
		for (int x = 0; x < 6; ++x) {
			const glm::ivec2 size(floor(m_renderSize.x / pow(2, x)), floor(m_renderSize.y / pow(2, x)));
			glTextureImage2DEXT(m_textureMipsID, GL_TEXTURE_2D, x, GL_RGB8, size.x, size.y, 0, GL_RGB, GL_FLOAT, NULL);
		}
		glNamedFramebufferTexture(m_fboMipsID, GL_COLOR_ATTACHMENT0, m_textureMipsID, 0);
		glNamedFramebufferDrawBuffer(m_fboMipsID, GL_COLOR_ATTACHMENT0);

		glCreateFramebuffers(1, &m_fboSSRID);
		glCreateTextures(GL_TEXTURE_2D, 1, &m_textureSSRID);
		glTextureImage2DEXT(m_textureSSRID, GL_TEXTURE_2D, 0, GL_RGB8, m_renderSize.x, m_renderSize.y, 0, GL_RGB, GL_FLOAT, NULL);
		glTextureParameteri(m_textureSSRID, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTextureParameteri(m_textureSSRID, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTextureParameteri(m_textureSSRID, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTextureParameteri(m_textureSSRID, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glNamedFramebufferTexture(m_fboSSRID, GL_COLOR_ATTACHMENT0, m_textureSSRID, 0);
		glNamedFramebufferDrawBuffer(m_fboSSRID, GL_COLOR_ATTACHMENT0);

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
		auto & msgMgr = m_engine->getManager_Messages();
		if (glCheckNamedFramebufferStatus(m_fboMipsID, GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			msgMgr.error("SSR Mipmap Framebuffer has encountered an error.");
		if (!glIsTexture(m_textureMipsID))
			msgMgr.error("SSR Mipmap Texture is incomplete.");
		if (glCheckNamedFramebufferStatus(m_fboSSRID, GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			msgMgr.error("SSR Framebuffer has encountered an error.");
		if (!glIsTexture(m_textureSSRID))
			msgMgr.error("SSR Texture is incomplete.");
		if (!glIsTexture(m_bayerID))
			msgMgr.error("SSR Bayer Matrix Texture is incomplete.");
	}


	// Public Interface Implementations.
	inline virtual void applyEffect(const float & deltaTime) override {
		if (!m_enabled || !m_shapeQuad->existsYet() || !m_shaderCopy->existsYet() || !m_shaderConvMips->existsYet() || !m_shaderSSR1->existsYet() || !m_shaderSSR2->existsYet())
			return;
		glBindVertexArray(m_shapeQuad->m_vaoID);
		m_quadIndirectBuffer.bindBuffer(GL_DRAW_INDIRECT_BUFFER);

		updateMIPChain();

		glDisable(GL_BLEND);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fboSSRID);
		m_geometryFBO->bindForReading();
		m_shaderSSR1->bind();
		glBindTextureUnit(6, m_bayerID);
		glDrawArraysIndirect(GL_TRIANGLES, 0);

		glEnable(GL_BLEND);
		glBlendEquation(GL_FUNC_ADD);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		m_reflectionFBO->bindForWriting();
		glBindTextureUnit(5, m_textureSSRID);
		glBindTextureUnit(6, m_textureMipsID);
		m_shaderSSR2->bind();
		glDrawArraysIndirect(GL_TRIANGLES, 0);
		glBlendFunc(GL_ONE, GL_ONE);
		glDisable(GL_BLEND);
	}


private:
	// Private Methods
	/** Convolute the lighting buffer into each of its mip levels. */
	inline void updateMIPChain() {
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_BLEND);

		// Copy lighting texture to one with a MIP chain
		m_shaderCopy->bind();
		m_lightingFBO->bindForReading();
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fboMipsID);
		GLfloat clearColor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
		glClearNamedFramebufferfv(m_fboMipsID, GL_COLOR, 0, clearColor);
		glDrawArraysIndirect(GL_TRIANGLES, 0);		

		// Blur MIP chain, reading from 1 MIP level and writing into next
		m_shaderConvMips->bind();
		glBindTextureUnit(0, m_textureMipsID);
		for (int horizontal = 0; horizontal < 2; ++horizontal) {
			m_shaderConvMips->setUniform(0, horizontal);
			glm::ivec2 read_size = m_renderSize;
			for (int x = 1; x < 6; ++x) {
				// Ensure we are reading from MIP level x - 1
				m_shaderConvMips->setUniform(1, read_size);
				glTextureParameteri(m_textureMipsID, GL_TEXTURE_BASE_LEVEL, x - 1);
				glTextureParameteri(m_textureMipsID, GL_TEXTURE_MAX_LEVEL, x - 1);
				// Ensure we are writing to MIP level x
				glm::ivec2 write_size(floor(m_renderSize.x / pow(2, x)), floor(m_renderSize.y / pow(2, x)));
				glNamedFramebufferTexture(m_fboMipsID, GL_COLOR_ATTACHMENT0, m_textureMipsID, x);

				glViewport(0, 0, std::max(1, write_size.x), std::max(1, write_size.y));
				glDrawArraysIndirect(GL_TRIANGLES, 0);
				read_size = write_size;
			}
			// Blend second pass
			glEnable(GL_BLEND);
			glBlendEquation(GL_FUNC_ADD);
			glBlendFunc(GL_ONE, GL_ONE);
		}

		// Restore to default
		glTextureParameteri(m_textureMipsID, GL_TEXTURE_BASE_LEVEL, 0);
		glTextureParameteri(m_textureMipsID, GL_TEXTURE_MAX_LEVEL, 5);
		glNamedFramebufferTexture(m_fboMipsID, GL_COLOR_ATTACHMENT0, m_textureMipsID, 0);
		glViewport(0, 0, m_renderSize.x, m_renderSize.y);
	}
	/** Resize the frame buffer.
	@param	size	the new size of the frame buffer */
	inline void resize(const glm::ivec2 & size) {
		m_renderSize = size;
		for (int x = 0; x < 6; ++x) {
			const glm::ivec2 mippedSize(floor(m_renderSize.x / pow(2, x)), floor(m_renderSize.y / pow(2, x)));
			glTextureImage2DEXT(m_textureMipsID, GL_TEXTURE_2D, x, GL_RGB8, mippedSize.x, mippedSize.y, 0, GL_RGB, GL_FLOAT, NULL);
		}
		glNamedFramebufferTexture(m_fboMipsID, GL_COLOR_ATTACHMENT0, m_textureMipsID, 0);
	}


	// Private Attributes
	Engine * m_engine = nullptr;
	FBO_Base * m_geometryFBO = nullptr, * m_lightingFBO = nullptr, * m_reflectionFBO = nullptr;
	Shared_Shader m_shaderSSR1, m_shaderSSR2, m_shaderCopy, m_shaderConvMips;
	Shared_Primitive m_shapeQuad;
	glm::ivec2 m_renderSize = glm::ivec2(1);
	GLuint m_fboMipsID = 0, m_textureMipsID = 0;
	GLuint m_fboSSRID = 0, m_textureSSRID = 0;
	GLuint m_bayerID = 0;
	StaticBuffer m_quadIndirectBuffer;
	std::shared_ptr<bool> m_aliveIndicator = std::make_shared<bool>(true);
};

#endif // SSR_H