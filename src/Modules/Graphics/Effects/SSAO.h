#pragma once
#ifndef SSAO_H
#define SSAO_H
#define MAX_KERNEL_SIZE 128

#include "Modules/Graphics/Common/Graphics_Technique.h"
#include "Modules/Graphics/Common/VisualFX.h"
#include "Assets/Shader.h"
#include "Assets/Primitive.h"
#include "Utilities/GL/StaticBuffer.h"
#include "Utilities/GL/FBO.h"
#include "Engine.h"
#include <random>


/** A core-rendering technique for deriving an ambient occlusion factor from the viewport itself. */
class SSAO : public Graphics_Technique {
public:
	// Public (de)Constructors
	/** Virtual Destructor. */
	inline ~SSAO() {
		// Update indicator
		m_aliveIndicator = false;
	}
	/** Constructor. */
	inline SSAO(Engine * engine, VisualFX * visualFX)
		: m_engine(engine), m_visualFX(visualFX) {
		// Asset Loading
		m_shader = Shared_Shader(m_engine, "Effects\\SSAO");
		m_shaderCopyAO = Shared_Shader(m_engine, "Effects\\SSAO To AO");
		m_shapeQuad = Shared_Primitive(m_engine, "quad");

		// Asset-Finished Callbacks
		m_shapeQuad->addCallback(m_aliveIndicator, [&]() mutable {
			const GLuint quadData[4] = { (GLuint)m_shapeQuad->getSize(), 1, 0, 0 }; // count, primCount, first, reserved
			m_quadIndirectBuffer = StaticBuffer(sizeof(GLuint) * 4, quadData, 0);
		});

		// Preferences
		auto & preferences = m_engine->getPreferenceState();
		preferences.getOrSetValue(PreferenceState::C_WINDOW_WIDTH, m_renderSize.x);
		preferences.addCallback(PreferenceState::C_WINDOW_WIDTH, m_aliveIndicator, [&](const float &f) { resize(glm::ivec2(f, m_renderSize.y)); });
		preferences.getOrSetValue(PreferenceState::C_WINDOW_HEIGHT, m_renderSize.y);
		preferences.addCallback(PreferenceState::C_WINDOW_HEIGHT, m_aliveIndicator, [&](const float &f) { resize(glm::ivec2(m_renderSize.x, f)); });
		preferences.getOrSetValue(PreferenceState::C_SSAO, m_enabled);
		preferences.addCallback(PreferenceState::C_SSAO, m_aliveIndicator, [&](const float &f) { m_enabled = (bool)f; });
		preferences.getOrSetValue(PreferenceState::C_SSAO_RADIUS, m_radius);
		preferences.addCallback(PreferenceState::C_SSAO_RADIUS, m_aliveIndicator, [&](const float &f) { m_radius = f; if (m_shader->existsYet()) m_shader->setUniform(0, m_radius); });
		preferences.getOrSetValue(PreferenceState::C_SSAO_QUALITY, m_quality);
		preferences.addCallback(PreferenceState::C_SSAO_QUALITY, m_aliveIndicator, [&](const float &f) { m_quality = (int)f; if (m_shader->existsYet()) m_shader->setUniform(1, m_quality); });
		preferences.getOrSetValue(PreferenceState::C_SSAO_BLUR_STRENGTH, m_blurStrength);
		preferences.addCallback(PreferenceState::C_SSAO_BLUR_STRENGTH, m_aliveIndicator, [&](const float &f) { m_blurStrength = (int)f; });
	
		// GL loading
		glCreateFramebuffers(1, &m_fboID);
		glCreateTextures(GL_TEXTURE_2D, 1, &m_textureID);
		glTextureImage2DEXT(m_textureID, GL_TEXTURE_2D, 0, GL_R8, m_renderSize.x, m_renderSize.y, 0, GL_RED, GL_FLOAT, NULL);
		glTextureParameteri(m_textureID, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTextureParameteri(m_textureID, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTextureParameteri(m_textureID, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTextureParameteri(m_textureID, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glNamedFramebufferTexture(m_fboID, GL_COLOR_ATTACHMENT0, m_textureID, 0);
		glNamedFramebufferDrawBuffer(m_fboID, GL_COLOR_ATTACHMENT0);
		glCreateTextures(GL_TEXTURE_2D, 2, m_textureIDSGB);
		for (int x = 0; x < 2; ++x) {
			glTextureImage2DEXT(m_textureIDSGB[x], GL_TEXTURE_2D, 0, GL_R8, m_renderSize.x, m_renderSize.y, 0, GL_RED, GL_FLOAT, NULL);
			glTextureParameteri(m_textureIDSGB[x], GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTextureParameteri(m_textureIDSGB[x], GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTextureParameteri(m_textureIDSGB[x], GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTextureParameteri(m_textureIDSGB[x], GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		}
		
		// Prepare the noise texture and kernal	
		std::uniform_real_distribution<GLfloat> randomFloats(0.0, 1.0);
		std::default_random_engine generator;
		glm::vec3 noiseArray[16];
		for (GLuint i = 0; i < 16; i++) {
			glm::vec3 noise(randomFloats(generator) * 2.0 - 1.0, randomFloats(generator) * 2.0 - 1.0, 0.0f);
			noiseArray[i] = (noise);
		}
		glCreateTextures(GL_TEXTURE_2D, 1, &m_noiseID);
		glTextureStorage2D(m_noiseID, 1, GL_RGB16F, 4, 4);
		glTextureSubImage2D(m_noiseID, 0, 0, 0, 4, 4, GL_RGB, GL_FLOAT, &noiseArray[0]);
		glTextureParameteri(m_noiseID, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTextureParameteri(m_noiseID, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTextureParameteri(m_noiseID, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTextureParameteri(m_noiseID, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		m_shader->addCallback(m_aliveIndicator, [&] {
			std::uniform_real_distribution<GLfloat> randomFloats(0.0, 1.0);
			std::default_random_engine generator;
			glm::vec4 new_kernel[MAX_KERNEL_SIZE];
			for (int i = 0, t = 0; i < MAX_KERNEL_SIZE; i++, t++) {
				glm::vec3 sample(
					randomFloats(generator) * 2.0 - 1.0,
					randomFloats(generator) * 2.0 - 1.0,
					randomFloats(generator)
				);
				sample = glm::normalize(sample);
				sample *= randomFloats(generator);
				GLfloat scale = GLfloat(i) / (GLfloat)(MAX_KERNEL_SIZE);
				scale = 0.1f + (scale*scale) * (1.0f - 0.1f);
				sample *= scale;
				new_kernel[t] = glm::vec4(sample, 1);
			}
			m_shader->setUniform(0, m_radius);
			m_shader->setUniform(1, m_quality);
			m_shader->setUniformArray(3, new_kernel, MAX_KERNEL_SIZE);
		});
		
		// Error Reporting
		auto & msgMgr = m_engine->getManager_Messages();
		if (glCheckNamedFramebufferStatus(m_fboID, GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			msgMgr.error("SSAO Framebuffer has encountered an error.");
		if (!glIsTexture(m_textureID))
			msgMgr.error("SSAO Texture is incomplete.");
		if (!glIsTexture(m_noiseID))
			msgMgr.error("SSAO Noise Texture is incomplete.");
	}


	// Public Interface Implementations.
	inline virtual void applyTechnique(const float & deltaTime) override {
		if (!m_enabled || !m_shapeQuad->existsYet() || !m_shader->existsYet() || !m_shaderCopyAO->existsYet())
			return;
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_BLEND);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fboID);
		m_shader->bind();
		m_gfxFBOS->bindForReading("GEOMETRY", 0);
		glBindTextureUnit(6, m_noiseID);
		glBindVertexArray(m_shapeQuad->m_vaoID);
		m_quadIndirectBuffer.bindBuffer(GL_DRAW_INDIRECT_BUFFER);
		glDrawArraysIndirect(GL_TRIANGLES, 0);

		// Gaussian blur the result
		m_visualFX->applyGaussianBlur_Alpha(m_textureID, m_textureIDSGB, m_renderSize, m_blurStrength);

		// Write result back to AO channel
		glEnable(GL_BLEND);
		glBlendFuncSeparate(GL_ONE, GL_ONE, GL_DST_ALPHA, GL_ZERO);
		m_shaderCopyAO->bind();
		m_gfxFBOS->bindForWriting("GEOMETRY");
		glDrawBuffer(GL_COLOR_ATTACHMENT2);
		glBindTextureUnit(0, m_textureID);
		glDrawArraysIndirect(GL_TRIANGLES, 0);
		GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
		glDrawBuffers(3, drawBuffers);
		glDisable(GL_BLEND);
	}


private:
	// Private Methods
	/** Resize the frame buffer.
	@param	size	the new size of the frame buffer */
	inline void resize(const glm::ivec2 & size) {
		m_renderSize = size;
		glTextureImage2DEXT(m_textureID, GL_TEXTURE_2D, 0, GL_R8, m_renderSize.x, m_renderSize.y, 0, GL_RED, GL_FLOAT, NULL);
		for (int x = 0; x < 2; ++x) 
			glTextureImage2DEXT(m_textureIDSGB[x], GL_TEXTURE_2D, 0, GL_R8, m_renderSize.x, m_renderSize.y, 0, GL_RED, GL_FLOAT, NULL);
	}


	// Private Attributes
	Engine * m_engine = nullptr;
	VisualFX * m_visualFX = nullptr;
	Shared_Shader m_shader, m_shaderCopyAO;
	Shared_Primitive m_shapeQuad;
	glm::ivec2 m_renderSize = glm::ivec2(1);
	float m_radius = 1.0f;
	int m_quality = 1, m_blurStrength = 5;
	GLuint m_fboID = 0, m_textureID = 0, m_textureIDSGB[2] = { 0,0 }, m_noiseID = 0;
	StaticBuffer m_quadIndirectBuffer;
	std::shared_ptr<bool> m_aliveIndicator = std::make_shared<bool>(true);
};

#endif // SSAO_H