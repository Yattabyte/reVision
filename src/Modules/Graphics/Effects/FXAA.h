#pragma once
#ifndef FXAA_H
#define FXAA_H

#include "Modules/Graphics/Common/Graphics_Technique.h"
#include "Assets/Shader.h"
#include "Assets/Auto_Model.h"
#include "Utilities/GL/DynamicBuffer.h"
#include "Utilities/GL/StaticTripleBuffer.h"
#include "Engine.h"


/** A post-processing technique for applying fxaa to the currently bound 2D image. */
class FXAA : public Graphics_Technique {
public:
	// Public (de)Constructors
	/** Virtual Destructor. */
	inline ~FXAA() {
		// Update indicator
		*m_aliveIndicator = false;
	}
	/** Constructor. */
	inline FXAA(Engine * engine) 
		: m_engine(engine), Graphics_Technique(POST_PROCESSING) {
		// Asset Loading
		m_shaderFXAA = Shared_Shader(m_engine, "Effects\\FXAA");
		m_shapeQuad = Shared_Auto_Model(m_engine, "quad");

		// Preferences
		auto & preferences = m_engine->getPreferenceState();
		preferences.getOrSetValue(PreferenceState::C_FXAA, m_enabled);
		preferences.addCallback(PreferenceState::C_FXAA, m_aliveIndicator, [&](const float &f) { m_enabled = (bool)f; });
	}


	// Public Interface Implementations.
	inline virtual void prepareForNextFrame(const float & deltaTime) override {
		for (auto &[camIndexBuffer, quadIndirectBuffer] : m_drawData) {
			camIndexBuffer.endWriting();
			quadIndirectBuffer.endWriting();
		}
		m_drawIndex = 0;
	}
	inline virtual void renderTechnique(const float & deltaTime, const std::shared_ptr<Viewport> & viewport, const std::vector<std::pair<int, int>> & perspectives) override {
		if (!m_enabled || !m_shapeQuad->existsYet() || !m_shaderFXAA->existsYet())
			return;

		// Prepare camera index
		if (m_drawIndex >= m_drawData.size())
			m_drawData.resize(size_t(m_drawIndex) + 1ull);
		auto &[camBufferIndex, quadIndirectBuffer] = m_drawData[m_drawIndex];
		camBufferIndex.beginWriting();
		quadIndirectBuffer.beginWriting();
		std::vector<glm::ivec2> camIndices;
		for (auto &[camIndex, layer] : perspectives)
			camIndices.push_back({ camIndex, layer });
		camBufferIndex.write(0, sizeof(glm::ivec2) * camIndices.size(), camIndices.data());
		const auto instanceCount = (GLuint)perspectives.size();
		quadIndirectBuffer.write(sizeof(GLuint), sizeof(GLuint), &instanceCount);
		camBufferIndex.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 3);

		// Apply FXAA effect
		viewport->m_gfxFBOS->bindForWriting("FXAA");
		viewport->m_gfxFBOS->bindForReading("HDR", 0);
		m_shaderFXAA->bind();
		glBindVertexArray(m_shapeQuad->m_vaoID);
		quadIndirectBuffer.bindBuffer(GL_DRAW_INDIRECT_BUFFER);
		glDrawArraysIndirect(GL_TRIANGLES, 0);

		// Bind for reading by next effect	
		glBindTextureUnit(0, viewport->m_gfxFBOS->getTexID("FXAA", 0));
		m_drawIndex++;
	}
	

private:
	// Private Attributes
	Engine * m_engine = nullptr;
	Shared_Shader m_shaderFXAA;
	Shared_Auto_Model m_shapeQuad;
	struct DrawData {
		DynamicBuffer camBufferIndex;
		constexpr static GLuint quadData[4] = { (GLuint)6, 1, 0, 0 };
		StaticTripleBuffer quadIndirectBuffer = StaticTripleBuffer(sizeof(GLuint) * 4, quadData);
	};
	std::vector<DrawData> m_drawData;
	int	m_drawIndex = 0;
	std::shared_ptr<bool> m_aliveIndicator = std::make_shared<bool>(true);
};

#endif // FXAA_H