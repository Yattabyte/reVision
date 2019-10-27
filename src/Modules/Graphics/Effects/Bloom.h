#pragma once
#ifndef BLOOM_H
#define BLOOM_H

#include "Modules/Graphics/Common/Graphics_Technique.h"
#include "Assets/Shader.h"
#include "Assets/Auto_Model.h"
#include "Utilities/GL/DynamicBuffer.h"
#include "Utilities/GL/IndirectDraw.h"
#include "Engine.h"


class Lighting_FBO;

/** A post-processing technique for generating bloom from a lighting buffer. */
class Bloom final : public Graphics_Technique {
public:
	// Public (de)Constructors
	/** Virtual Destructor. */
	inline ~Bloom() {
		// Update indicator
		*m_aliveIndicator = false;
	}
	/** Constructor. */
	inline explicit Bloom(Engine* engine)
		: m_engine(engine), Graphics_Technique(POST_PROCESSING) {
		// Asset Loading
		m_shaderBloomExtract = Shared_Shader(engine, "Effects\\Bloom Extraction");
		m_shaderCopy = Shared_Shader(engine, "Effects\\Copy Texture");
		m_shaderGB = Shared_Shader(engine, "Effects\\Gaussian Blur");
		m_shapeQuad = Shared_Auto_Model(engine, "quad");

		// Preference Callbacks
		auto& preferences = m_engine->getPreferenceState();
		preferences.getOrSetValue(PreferenceState::C_BLOOM, m_enabled);
		preferences.addCallback(PreferenceState::C_BLOOM, m_aliveIndicator, [&](const float& f) { m_enabled = (bool)f; });
		preferences.getOrSetValue(PreferenceState::C_BLOOM_STRENGTH, m_bloomStrength);
		preferences.addCallback(PreferenceState::C_BLOOM_STRENGTH, m_aliveIndicator, [&](const float& f) { setBloomStrength((int)f); });
	}


	// Public Interface Implementations.
	inline virtual void prepareForNextFrame(const float& deltaTime) override final {
		m_drawIndex = 0;
	}
	inline virtual void renderTechnique(const float& deltaTime, const std::shared_ptr<Viewport>& viewport, const std::vector<std::pair<int, int>>& perspectives) override final {
		if (!m_enabled || !m_shapeQuad->existsYet() || !m_shaderBloomExtract->existsYet() || !m_shaderCopy->existsYet() || !m_shaderGB->existsYet())
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

		// Extract bright regions from lighting buffer
		camBufferIndex.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 3);
		m_shaderBloomExtract->bind();
		viewport->m_gfxFBOS->bindForWriting("BLOOM");
		viewport->m_gfxFBOS->bindForReading("LIGHTING", 0);
		glBindVertexArray(m_shapeQuad->m_vaoID);
		glDrawBuffer(GL_COLOR_ATTACHMENT0);
		indirectQuad.bind();
		glDrawArraysIndirect(GL_TRIANGLES, 0);
		size_t bloomSpot = 0;

		if (m_bloomStrength > 0) {
			// Read from desired texture, blur into this frame buffer
			bool horizontal = false;
			glBindTextureUnit(0, viewport->m_gfxFBOS->getTexID("BLOOM", 0));
			glBindTextureUnit(1, viewport->m_gfxFBOS->getTexID("BLOOM", 1));
			glDrawBuffer(GL_COLOR_ATTACHMENT0 + horizontal);
			m_shaderGB->bind();
			m_shaderGB->setUniform(0, horizontal);
			m_shaderGB->setUniform(1, glm::vec2(viewport->m_dimensions));

			// Blur remainder of the times
			for (int i = 0; i < m_bloomStrength; i++) {
				horizontal = !horizontal;
				glDrawBuffer(GL_COLOR_ATTACHMENT0 + horizontal);
				m_shaderGB->setUniform(0, horizontal);
				glDrawArraysIndirect(GL_TRIANGLES, 0);
			}
			bloomSpot = horizontal ? 1ull : 0ull;
		}

		// Copy to lighting buffer
		glEnable(GL_BLEND);
		glBlendEquation(GL_FUNC_ADD);
		glBlendFunc(GL_ONE, GL_ONE);
		viewport->m_gfxFBOS->bindForWriting("LIGHTING");
		glBindTextureUnit(0, viewport->m_gfxFBOS->getTexID("BLOOM", bloomSpot));
		m_shaderCopy->bind();
		glDrawArraysIndirect(GL_TRIANGLES, 0);
		glDisable(GL_BLEND);
		camBufferIndex.endReading();
		indirectQuad.endReading();
		Shader::Release();
		m_drawIndex++;
	}


private:
	// Private Methods
	/** Change the strength of the bloom effect.
	@param	strength		the new strength of the bloom effect. */
	inline void setBloomStrength(const int& strength) {
		m_bloomStrength = strength;
	}


	// Private Attributes
	Engine* m_engine = nullptr;
	Shared_Shader m_shaderBloomExtract, m_shaderCopy, m_shaderGB;
	Shared_Auto_Model m_shapeQuad;
	int m_bloomStrength = 5;
	struct DrawData {
		DynamicBuffer<> camBufferIndex;
		IndirectDraw<> indirectQuad = IndirectDraw((GLuint)6, 1, 0, GL_DYNAMIC_STORAGE_BIT);
	};
	std::vector<DrawData> m_drawData;
	int	m_drawIndex = 0;
	std::shared_ptr<bool> m_aliveIndicator = std::make_shared<bool>(true);
};

#endif // BLOOM_H