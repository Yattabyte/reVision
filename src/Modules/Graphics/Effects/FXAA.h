#pragma once
#ifndef FXAA_H
#define FXAA_H

#include "Modules/Graphics/Common/Graphics_Technique.h"
#include "Assets/Shader.h"
#include "Assets/Auto_Model.h"
#include "Utilities/GL/DynamicBuffer.h"
#include "Utilities/GL/IndirectDraw.h"
#include "Engine.h"


/** A post-processing technique for applying fxaa to the currently bound 2D image. */
class FXAA final : public Graphics_Technique {
public:
	// Public (de)Constructors
	/** Virtual Destructor. */
	inline ~FXAA() {
		// Update indicator
		*m_aliveIndicator = false;
	}
	/** Constructor. */
	inline explicit FXAA(Engine* engine)
		: m_engine(engine), Graphics_Technique(POST_PROCESSING) {
		// Asset Loading
		m_shaderFXAA = Shared_Shader(engine, "Effects\\FXAA");
		m_shapeQuad = Shared_Auto_Model(engine, "quad");

		// Preferences
		auto& preferences = m_engine->getPreferenceState();
		preferences.getOrSetValue(PreferenceState::C_FXAA, m_enabled);
		preferences.addCallback(PreferenceState::C_FXAA, m_aliveIndicator, [&](const float& f) { m_enabled = (bool)f; });
	}


	// Public Interface Implementations.
	inline virtual void clearCache(const float& deltaTime) override final {
		m_drawIndex = 0;
	}
	inline virtual void renderTechnique(const float& deltaTime, const std::shared_ptr<Viewport>& viewport, const std::vector<std::pair<int, int>>& perspectives) override final {
		if (!m_enabled || !m_shapeQuad->existsYet() || !m_shaderFXAA->existsYet())
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

		// Apply FXAA effect
		camBufferIndex.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 3);
		viewport->m_gfxFBOS->bindForWriting("FXAA");
		viewport->m_gfxFBOS->bindForReading("HDR", 0);
		m_shaderFXAA->bind();
		glBindVertexArray(m_shapeQuad->m_vaoID);
		indirectQuad.drawCall();

		// Bind for reading by next effect
		glBindTextureUnit(0, viewport->m_gfxFBOS->getTexID("FXAA", 0));
		camBufferIndex.endReading();
		indirectQuad.endReading();
		Shader::Release();
		m_drawIndex++;
	}


private:
	// Private Attributes
	Engine* m_engine = nullptr;
	Shared_Shader m_shaderFXAA;
	Shared_Auto_Model m_shapeQuad;
	struct DrawData {
		DynamicBuffer<> camBufferIndex;
		IndirectDraw<> indirectQuad = IndirectDraw((GLuint)6, 1, 0, GL_DYNAMIC_STORAGE_BIT);
	};
	std::vector<DrawData> m_drawData;
	int	m_drawIndex = 0;
	std::shared_ptr<bool> m_aliveIndicator = std::make_shared<bool>(true);
};

#endif // FXAA_H