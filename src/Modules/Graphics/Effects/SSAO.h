#pragma once
#ifndef SSAO_H
#define SSAO_H
#define MAX_KERNEL_SIZE 128

#include "Modules/Graphics/Common/Graphics_Technique.h"
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
	inline SSAO(Engine * engine)
		: m_engine(engine), Graphics_Technique(SECONDARY_LIGHTING) {
		// Asset Loading
		m_shader = Shared_Shader(m_engine, "Effects\\SSAO");
		m_shaderCopyAO = Shared_Shader(m_engine, "Effects\\SSAO To AO");
		m_shaderGB_A = Shared_Shader(m_engine, "Effects\\Gaussian Blur Alpha");
		m_shapeQuad = Shared_Primitive(m_engine, "quad");

		// Asset-Finished Callbacks
		m_shapeQuad->addCallback(m_aliveIndicator, [&]() mutable {
			const GLuint quadData[4] = { (GLuint)m_shapeQuad->getSize(), 1, 0, 0 }; // count, primCount, first, reserved
			m_quadIndirectBuffer = StaticBuffer(sizeof(GLuint) * 4, quadData, 0);
		});

		// Preferences
		auto & preferences = m_engine->getPreferenceState();
		preferences.getOrSetValue(PreferenceState::C_SSAO, m_enabled);
		preferences.addCallback(PreferenceState::C_SSAO, m_aliveIndicator, [&](const float &f) { m_enabled = (bool)f; });
		preferences.getOrSetValue(PreferenceState::C_SSAO_RADIUS, m_radius);
		preferences.addCallback(PreferenceState::C_SSAO_RADIUS, m_aliveIndicator, [&](const float &f) { m_radius = f; if (m_shader->existsYet()) m_shader->setUniform(0, m_radius); });
		preferences.getOrSetValue(PreferenceState::C_SSAO_QUALITY, m_quality);
		preferences.addCallback(PreferenceState::C_SSAO_QUALITY, m_aliveIndicator, [&](const float &f) { m_quality = (int)f; if (m_shader->existsYet()) m_shader->setUniform(1, m_quality); });
		preferences.getOrSetValue(PreferenceState::C_SSAO_BLUR_STRENGTH, m_blurStrength);
		preferences.addCallback(PreferenceState::C_SSAO_BLUR_STRENGTH, m_aliveIndicator, [&](const float &f) { m_blurStrength = (int)f; });
			
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
		if (!glIsTexture(m_noiseID))
			msgMgr.error("SSAO Noise Texture is incomplete.");
	}


	// Public Interface Implementations.
	inline virtual void renderTechnique(const float & deltaTime, const std::shared_ptr<Viewport> & viewport, const std::shared_ptr<CameraBuffer> & camera) override {
		if (!m_enabled || !m_shapeQuad->existsYet() || !m_shader->existsYet() || !m_shaderCopyAO->existsYet() && m_shaderGB_A->existsYet())
			return;
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_BLEND);
		m_shader->bind();
		viewport->m_gfxFBOS->bindForWriting("SSAO");
		viewport->m_gfxFBOS->bindForReading("GEOMETRY", 0);
		glBindTextureUnit(6, m_noiseID);
		glBindVertexArray(m_shapeQuad->m_vaoID);
		m_quadIndirectBuffer.bindBuffer(GL_DRAW_INDIRECT_BUFFER);
		glDrawBuffer(GL_COLOR_ATTACHMENT0);
		glDrawArraysIndirect(GL_TRIANGLES, 0);
		size_t aoSpot = 0;
			
		// Gaussian blur the lines we got from the SSAO pass
		if (m_blurStrength > 0) {
			// Clear the second attachment
			GLfloat clearRed = 0.0f;
			glClearTexImage(viewport->m_gfxFBOS->getTexID("SSAO", 1), 0, GL_RED, GL_FLOAT, &clearRed);

			// Read from desired texture, blur into this frame buffer
			bool horizontal = false;
			glBindTextureUnit(0, viewport->m_gfxFBOS->getTexID("SSAO", 0));
			glBindTextureUnit(1, viewport->m_gfxFBOS->getTexID("SSAO", 1));
			glDrawBuffer(GL_COLOR_ATTACHMENT0 + horizontal);
			m_shaderGB_A->bind();
			m_shaderGB_A->setUniform(0, horizontal);
			m_shaderGB_A->setUniform(1, glm::vec2(viewport->m_dimensions));
			glBindVertexArray(m_shapeQuad->m_vaoID);
			m_quadIndirectBuffer.bindBuffer(GL_DRAW_INDIRECT_BUFFER);

			// Blur remainder of the times
			for (int i = 0; i < m_blurStrength; i++) {
				horizontal = !horizontal;
				glDrawBuffer(GL_COLOR_ATTACHMENT0 + horizontal);
				m_shaderGB_A->setUniform(0, horizontal);
				glDrawArraysIndirect(GL_TRIANGLES, 0);
			}
			aoSpot = horizontal ? 1ull : 0ull;
		}		

		// Overlay SSAO on top of AO channel of Geometry Buffer
		glEnable(GL_BLEND);
		glBlendFuncSeparate(GL_ONE, GL_ONE, GL_DST_ALPHA, GL_ZERO);
		m_shaderCopyAO->bind();
		viewport->m_gfxFBOS->bindForWriting("GEOMETRY");
		glDrawBuffer(GL_COLOR_ATTACHMENT2);
		glBindTextureUnit(0, viewport->m_gfxFBOS->getTexID("SSAO", aoSpot));
		glDrawArraysIndirect(GL_TRIANGLES, 0);
		GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
		glDrawBuffers(3, drawBuffers);
		glDisable(GL_BLEND);
	}


private:
	// Private Attributes
	Engine * m_engine = nullptr;
	Shared_Shader m_shader, m_shaderCopyAO, m_shaderGB_A;
	Shared_Primitive m_shapeQuad;
	float m_radius = 1.0f;
	int m_quality = 1, m_blurStrength = 5;
	GLuint m_noiseID = 0;
	StaticBuffer m_quadIndirectBuffer;
	std::shared_ptr<bool> m_aliveIndicator = std::make_shared<bool>(true);
};

#endif // SSAO_H