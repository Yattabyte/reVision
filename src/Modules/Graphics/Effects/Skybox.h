#pragma once
#ifndef SKYBOX_H
#define SKYBOX_H

#include "Modules\Graphics\Effects\GFX_Core_Effect.h"
#include "Assets\Asset_Shader.h"
#include "Assets\Asset_Cubemap.h"
#include "Assets\Asset_Primitive.h"
#include "Utilities\GL\FBO.h"
#include "Utilities\GL\StaticBuffer.h"
#include "Engine.h"


/** A core-rendering technique for writing the frame time to the screen. */
class Skybox : public GFX_Core_Effect {
public:
	// (de)Constructors
	/** Virtual Destructor. */
	~Skybox() {
		// Update indicator
		m_aliveIndicator = false;
	}
	/** Constructor. */
	Skybox(Engine * engine, FBO_Base * geometryFBO, FBO_Base * lightingFBO, FBO_Base * reflectionFBO
	) : m_engine(engine), m_geometryFBO(geometryFBO), m_lightingFBO(lightingFBO), m_reflectionFBO(reflectionFBO) {
		// Asset Loading
		m_cubemapSky = Asset_Cubemap::Create(engine, "none\\");
		m_shaderSky = Asset_Shader::Create(engine, "Effects\\Skybox");
		m_shaderSkyReflect = Asset_Shader::Create(engine, "Effects\\Skybox Reflection");
		m_shaderConvolute = Asset_Shader::Create(engine, "Effects\\Sky_Convolution");
		m_shapeQuad = Asset_Primitive::Create(engine, "quad");

		glCreateFramebuffers(1, &m_cubeFBO);
		glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &m_cubemapMipped);
		glTextureParameteri(m_cubemapMipped, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTextureParameteri(m_cubemapMipped, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTextureParameteri(m_cubemapMipped, GL_TEXTURE_MIN_LOD, 0);
		glTextureParameteri(m_cubemapMipped, GL_TEXTURE_MAX_LOD, 5);
		glTextureParameteri(m_cubemapMipped, GL_TEXTURE_BASE_LEVEL, 0);
		glTextureParameteri(m_cubemapMipped, GL_TEXTURE_MAX_LEVEL, 5);
		glTextureParameteri(m_cubemapMipped, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTextureParameteri(m_cubemapMipped, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTextureParameteri(m_cubemapMipped, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

		// Preferences
		auto & preferences = m_engine->getPreferenceState();
		preferences.getOrSetValue(PreferenceState::C_WINDOW_WIDTH, m_renderSize.x);
		preferences.addCallback(PreferenceState::C_WINDOW_WIDTH, m_aliveIndicator, [&](const float &f) {m_renderSize = glm::ivec2(f, m_renderSize.y); });
		preferences.getOrSetValue(PreferenceState::C_WINDOW_HEIGHT, m_renderSize.y);
		preferences.addCallback(PreferenceState::C_WINDOW_HEIGHT, m_aliveIndicator, [&](const float &f) {m_renderSize = glm::ivec2(m_renderSize.x, f); });

		// Asset-Finished Callbacks
		m_shapeQuad->addCallback(m_aliveIndicator, [&]() mutable {
			const GLuint quadData[4] = { (GLuint)m_shapeQuad->getSize(), 1, 0, 0 }; // count, primCount, first, reserved
			m_quadIndirectBuffer = StaticBuffer(sizeof(GLuint) * 4, quadData, 0);
			const GLuint quad6Data[4] = { (GLuint)m_shapeQuad->getSize(), 6, 0, 0 };
			m_quad6IndirectBuffer = StaticBuffer(sizeof(GLuint) * 4, quad6Data, 0);
		});
		m_cubemapSky->addCallback(m_aliveIndicator, [&](void) mutable {
			m_skyOutOfDate = true;
			m_skySize = m_cubemapSky->m_images[0]->m_size;
			glTextureStorage2D(m_cubemapMipped, 6, GL_RGB16F, m_skySize.x, m_skySize.x);
			for (int x = 0; x < 6; ++x) {
				glBindBuffer(GL_PIXEL_UNPACK_BUFFER, m_cubemapSky->m_pboIDs[x]);
				glTextureSubImage3D(m_cubemapMipped, 0, 0, 0, x, m_skySize.x, m_skySize.x, 1, GL_RGBA, GL_UNSIGNED_BYTE, (void*)0);
			}
			glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
			glNamedFramebufferTexture(m_cubeFBO, GL_COLOR_ATTACHMENT0, m_cubemapMipped, 0);
			glNamedFramebufferDrawBuffer(m_cubeFBO, GL_COLOR_ATTACHMENT0);
			const GLenum Status = glCheckNamedFramebufferStatus(m_cubeFBO, GL_FRAMEBUFFER);
			//if (Status != GL_FRAMEBUFFER_COMPLETE && Status != GL_NO_ERROR)
			//	m_engine->getMessageManager().error("Skybox Framebuffer is incomplete. Reason: \n" + std::string(reinterpret_cast<char const *>(glewGetErrorString(Status))));
			if (!glIsTexture(m_cubemapMipped))
				m_engine->getMessageManager().error("Skybox Texture is incomplete.");
		});
	}


	// Interface Implementations.
	inline virtual void applyEffect(const float & deltaTime) override {
		if (!m_shapeQuad->existsYet() || !m_shaderSky->existsYet() || !m_shaderSkyReflect->existsYet() || !m_shaderConvolute->existsYet() || !m_cubemapSky->existsYet())
			return;
		if (m_skyOutOfDate ) {
			convoluteSky();
			m_skyOutOfDate = false;
		}
		glDisable(GL_BLEND);
		glDepthMask(GL_FALSE);
		glEnable(GL_DEPTH_TEST);
		glBindVertexArray(m_shapeQuad->m_vaoID);
		m_quadIndirectBuffer.bindBuffer(GL_DRAW_INDIRECT_BUFFER);
		m_geometryFBO->bindForReading();
		glBindTextureUnit(4, m_cubemapMipped);

		// Render skybox to reflection buffer
		// Uses the stencil buffer, last thing to write to it should be the reflector pass
		glEnable(GL_STENCIL_TEST);
		glStencilFunc(GL_EQUAL, 0, 0xFF);
		m_shaderSkyReflect->bind();
		m_reflectionFBO->bindForWriting();
		glDrawArraysIndirect(GL_TRIANGLES, 0);
		glDisable(GL_STENCIL_TEST);

		// Render skybox to lighting buffer
		m_shaderSky->bind();
		m_lightingFBO->bindForWriting();
		glDrawArraysIndirect(GL_TRIANGLES, 0);

		glEnable(GL_BLEND);
		glDepthMask(GL_TRUE);
		glDisable(GL_DEPTH_TEST);
	}


private:
	// Private Methods
	inline void convoluteSky() {
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_BLEND);
		glDepthMask(GL_FALSE);
		glBindVertexArray(m_shapeQuad->m_vaoID);
		m_shaderConvolute->bind();
		m_shaderConvolute->setUniform(0, 0);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_cubeFBO);
		glBindTextureUnit(0, m_cubemapMipped);
		m_quad6IndirectBuffer.bindBuffer(GL_DRAW_INDIRECT_BUFFER);
		for (unsigned int r = 1; r < 6; ++r) {
			// Ensure we are writing to MIP level r
			const unsigned int write_size = (unsigned int)std::max(1.0f, (floor((float)m_skySize.x / pow(2.0f, (float)r))));
			glViewport(0, 0, write_size, write_size);
			m_shaderConvolute->setUniform(1, (float)r / 5.0f);
			glNamedFramebufferTexture(m_cubeFBO, GL_COLOR_ATTACHMENT0, m_cubemapMipped, r);

			// Ensure we are reading from MIP level r - 1
			glTextureParameteri(m_cubemapMipped, GL_TEXTURE_BASE_LEVEL, r - 1);
			glTextureParameteri(m_cubemapMipped, GL_TEXTURE_MAX_LEVEL, r - 1);

			// Convolute the 6 faces for this roughness level (RENDERS 6 TIMES)
			glDrawArraysIndirect(GL_TRIANGLES, 0);
		}

		// Reset
		glTextureParameteri(m_cubemapMipped, GL_TEXTURE_BASE_LEVEL, 0);
		glTextureParameteri(m_cubemapMipped, GL_TEXTURE_MAX_LEVEL, 5);
		glNamedFramebufferTexture(m_cubeFBO, GL_COLOR_ATTACHMENT0, m_cubemapMipped, 0);
		glViewport(0, 0, m_renderSize.x, m_renderSize.y);
		Asset_Shader::Release();
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	}


	// Private Attributes
	Engine * m_engine = nullptr;
	FBO_Base * m_geometryFBO = nullptr, * m_lightingFBO = nullptr, * m_reflectionFBO = nullptr;
	GLuint m_cubeFBO = 0, m_cubemapMipped = 0;
	Shared_Asset_Cubemap m_cubemapSky;
	Shared_Asset_Shader m_shaderSky, m_shaderSkyReflect, m_shaderConvolute;
	Shared_Asset_Primitive m_shapeQuad;
	StaticBuffer m_quadIndirectBuffer, m_quad6IndirectBuffer;
	bool m_skyOutOfDate = false;
	glm::ivec2 m_renderSize = glm::ivec2(1), m_skySize = glm::ivec2(1);
	std::shared_ptr<bool> m_aliveIndicator = std::make_shared<bool>(true);
};

#endif // FRAMETIME_COUNTER_H