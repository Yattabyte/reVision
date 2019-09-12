#pragma once
#ifndef HDR_H
#define HDR_H

#include "Modules/Graphics/Common/Graphics_Technique.h"
#include "Assets/Shader.h"
#include "Assets/Auto_Model.h"
#include "Utilities/GL/DynamicBuffer.h"
#include "Utilities/GL/IndirectDraw.h"
#include "Engine.h"


/** A post-processing technique for tone-mapping and gamma correcting the final lighting product. */
class HDR : public Graphics_Technique {
public:
	// Public (de)Constructors
	/** Virtual Destructor. */
	inline ~HDR() {
		// Update indicator
		*m_aliveIndicator = false;
	}
	/** Constructor. */
	inline HDR(Engine * engine)
		: m_engine(engine), Graphics_Technique(POST_PROCESSING) {
		// Asset Loading
		m_shaderHDR = Shared_Shader(engine, "Effects\\HDR");
		m_shapeQuad = Shared_Auto_Model(engine, "quad");

		// Preferences
		auto & preferences = m_engine->getPreferenceState();
		preferences.getOrSetValue(PreferenceState::C_GAMMA, m_gamma);
		preferences.addCallback(PreferenceState::C_GAMMA, m_aliveIndicator, [&](const float &f) { m_gamma = f; });
	}


	// Public Interface Implementations.
	inline virtual void prepareForNextFrame(const float & deltaTime) override {
		for (auto &[camIndexBuffer, indirectQuad] : m_drawData) {
			camIndexBuffer.endWriting();
			indirectQuad.endWriting();
		}
		m_drawIndex = 0;
	}
	inline virtual void renderTechnique(const float & deltaTime, const std::shared_ptr<Viewport> & viewport, const std::vector<std::pair<int, int>> & perspectives) override {
		if (!m_enabled || !m_shapeQuad->existsYet() || !m_shaderHDR->existsYet())
			return;
		
		// Prepare camera index
		if (m_drawIndex >= m_drawData.size())
			m_drawData.resize(size_t(m_drawIndex) + 1ull);
		auto &[camBufferIndex, indirectQuad] = m_drawData[m_drawIndex];
		camBufferIndex.beginWriting();
		indirectQuad.beginWriting();
		std::vector<glm::ivec2> camIndices;
		for (auto &[camIndex, layer] : perspectives)
			camIndices.push_back({ camIndex, layer });
		camBufferIndex.write(0, sizeof(glm::ivec2) * camIndices.size(), camIndices.data());
		indirectQuad.setPrimitiveCount((GLuint)perspectives.size());
		camBufferIndex.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 3);

		// Write HDR effect to own framebuffer
		viewport->m_gfxFBOS->bindForWriting("HDR");
		viewport->m_gfxFBOS->bindForReading("LIGHTING", 0);
		m_shaderHDR->bind();
		m_shaderHDR->setUniform(0, 1.0f);
		m_shaderHDR->setUniform(1, m_gamma);
		// Use the currently bound framebuffer from the prior effect
		glBindVertexArray(m_shapeQuad->m_vaoID);
		indirectQuad.drawCall();
		m_drawIndex++;
	}


private:
	// Private Attributes
	Engine * m_engine = nullptr;
	float m_gamma = 1.0f;
	Shared_Shader m_shaderHDR;
	Shared_Auto_Model m_shapeQuad;
	struct DrawData {
		DynamicBuffer camBufferIndex;
		IndirectDraw indirectQuad = IndirectDraw((GLuint)6, 1, 0, GL_DYNAMIC_STORAGE_BIT);
	};
	std::vector<DrawData> m_drawData;
	int	m_drawIndex = 0;
	std::shared_ptr<bool> m_aliveIndicator = std::make_shared<bool>(true);
};

#endif // HDR_H