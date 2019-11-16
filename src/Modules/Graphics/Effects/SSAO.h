#pragma once
#ifndef SSAO_H
#define SSAO_H
#define MAX_KERNEL_SIZE 128

#include "Modules/Graphics/Common/Graphics_Technique.h"
#include "Assets/Shader.h"
#include "Assets/Auto_Model.h"
#include "Utilities/GL/DynamicBuffer.h"
#include "Utilities/GL/IndirectDraw.h"
#include "Engine.h"
#include <random>


/** A core-rendering technique for deriving an ambient occlusion factor from the viewport itself. */
class SSAO final : public Graphics_Technique {
public:
	// Public (De)Constructors
	/** Virtual Destructor. */
	inline ~SSAO() noexcept {
		// Update indicator
		*m_aliveIndicator = false;
	}
	/** Constructor. */
	inline explicit SSAO(Engine* engine) noexcept :
		Graphics_Technique(Technique_Category::SECONDARY_LIGHTING),
		m_engine(engine),
		m_shader(Shared_Shader(engine, "Effects\\SSAO")),
		m_shaderCopyAO(Shared_Shader(engine, "Effects\\SSAO To AO")),
		m_shaderGB_A(Shared_Shader(engine, "Effects\\Gaussian Blur Alpha")),
		m_shapeQuad(Shared_Auto_Model(engine, "quad"))
	{
		// Preferences
		auto& preferences = m_engine->getPreferenceState();
		preferences.getOrSetValue(PreferenceState::Preference::C_SSAO, m_enabled);
		preferences.addCallback(PreferenceState::Preference::C_SSAO, m_aliveIndicator, [&](const float& f) { m_enabled = (bool)f; });
		preferences.getOrSetValue(PreferenceState::Preference::C_SSAO_RADIUS, m_radius);
		preferences.addCallback(PreferenceState::Preference::C_SSAO_RADIUS, m_aliveIndicator, [&](const float& f) { m_radius = f; if (m_shader->existsYet()) m_shader->setUniform(0, m_radius); });
		preferences.getOrSetValue(PreferenceState::Preference::C_SSAO_QUALITY, m_quality);
		preferences.addCallback(PreferenceState::Preference::C_SSAO_QUALITY, m_aliveIndicator, [&](const float& f) { m_quality = (int)f; if (m_shader->existsYet()) m_shader->setUniform(1, m_quality); });
		preferences.getOrSetValue(PreferenceState::Preference::C_SSAO_BLUR_STRENGTH, m_blurStrength);
		preferences.addCallback(PreferenceState::Preference::C_SSAO_BLUR_STRENGTH, m_aliveIndicator, [&](const float& f) { m_blurStrength = (int)f; });

		// Prepare the noise texture and kernel
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
				scale = 0.1f + (scale * scale) * (1.0f - 0.1f);
				sample *= scale;
				new_kernel[t] = glm::vec4(sample, 1);
			}
			m_shader->setUniform(0, m_radius);
			m_shader->setUniform(1, m_quality);
			m_shader->setUniformArray(3, new_kernel, MAX_KERNEL_SIZE);
			});

		// Error Reporting
		auto& msgMgr = m_engine->getManager_Messages();
		if (!glIsTexture(m_noiseID))
			msgMgr.error("SSAO Noise Texture is incomplete.");
	}


	// Public Interface Implementations.
	inline virtual void clearCache(const float& deltaTime) noexcept override final {
		m_drawIndex = 0;
	}
	inline virtual void renderTechnique(const float& deltaTime, const std::shared_ptr<Viewport>& viewport, const std::vector<std::pair<int, int>>& perspectives) noexcept override final {
		if (!m_enabled || !m_shapeQuad->existsYet() || !m_shader->existsYet() || !m_shaderCopyAO->existsYet() && m_shaderGB_A->existsYet())
			return;

		// Prepare camera index
		if (m_drawIndex >= m_drawData.size())
			m_drawData.resize(size_t(m_drawIndex) + 1ull);
		auto& [camBufferIndex, indirectQuad] = m_drawData[m_drawIndex];
		camBufferIndex.beginWriting();
		indirectQuad.beginWriting();
		std::vector<glm::ivec2> camIndices;
		for (auto& [camIndex, layer] : perspectives)
			camIndices.push_back({ camIndex, layer });
		camBufferIndex.write(0, sizeof(glm::ivec2) * camIndices.size(), camIndices.data());
		indirectQuad.setPrimitiveCount((GLuint)perspectives.size());
		camBufferIndex.endWriting();
		indirectQuad.endWriting();

		// Bind common data
		camBufferIndex.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 3);
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_BLEND);
		m_shader->bind();
		viewport->m_gfxFBOS->bindForWriting("SSAO");
		viewport->m_gfxFBOS->bindForReading("GEOMETRY", 0);
		glBindTextureUnit(6, m_noiseID);
		glBindVertexArray(m_shapeQuad->m_vaoID);
		glDrawBuffer(GL_COLOR_ATTACHMENT0);
		indirectQuad.bind();
		glDrawArraysIndirect(GL_TRIANGLES, 0);

		// Gaussian blur the lines we got from the SSAO pass
		size_t aoSpot = 0;
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
		camBufferIndex.endReading();
		indirectQuad.endReading();
		Shader::Release();
		m_drawIndex++;
	}


private:
	// Private Attributes
	Engine* m_engine = nullptr;
	Shared_Shader m_shader, m_shaderCopyAO, m_shaderGB_A;
	Shared_Auto_Model m_shapeQuad;
	float m_radius = 1.0f;
	int m_quality = 1, m_blurStrength = 5;
	GLuint m_noiseID = 0;
	struct DrawData {
		DynamicBuffer<> camBufferIndex;
		IndirectDraw<> indirectQuad = IndirectDraw((GLuint)6, 1, 0, GL_DYNAMIC_STORAGE_BIT);
	};
	std::vector<DrawData> m_drawData;
	int	m_drawIndex = 0;
	std::shared_ptr<bool> m_aliveIndicator = std::make_shared<bool>(true);
};

#endif // SSAO_H
