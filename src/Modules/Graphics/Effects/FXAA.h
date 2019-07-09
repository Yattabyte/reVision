#pragma once
#ifndef FXAA_H
#define FXAA_H

#include "Modules/Graphics/Common/Graphics_Technique.h"
#include "Assets/Shader.h"
#include "Assets/Primitive.h"
#include "Utilities/GL/DynamicBuffer.h"
#include "Utilities/GL/StaticBuffer.h"
#include "Engine.h"


/** A post-processing technique for applying fxaa to the currently bound 2D image. */
class FXAA : public Graphics_Technique {
public:
	// Public (de)Constructors
	/** Virtual Destructor. */
	inline ~FXAA() {
		// Update indicator
		m_aliveIndicator = false;
	}
	/** Constructor. */
	inline FXAA(Engine * engine) 
		: m_engine(engine), Graphics_Technique(POST_PROCESSING) {
		// Asset Loading
		m_shaderFXAA = Shared_Shader(m_engine, "Effects\\FXAA");
		m_shapeQuad = Shared_Primitive(m_engine, "quad");

		// Asset-Finished Callbacks
		m_shapeQuad->addCallback(m_aliveIndicator, [&]() mutable {
			const GLuint quadData[4] = { (GLuint)m_shapeQuad->getSize(), 1, 0, 0 }; // count, primCount, first, reserved
			m_quadIndirectBuffer = StaticBuffer(sizeof(GLuint) * 4, quadData);
		});

		// Preferences
		auto & preferences = m_engine->getPreferenceState();
		preferences.getOrSetValue(PreferenceState::C_FXAA, m_enabled);
		preferences.addCallback(PreferenceState::C_FXAA, m_aliveIndicator, [&](const float &f) { m_enabled = (bool)f; });
	}


	// Public Interface Implementations.
	inline virtual void prepareForNextFrame(const float & deltaTime) override {
		for (auto & camIndexBuffer : m_camIndexes)
			camIndexBuffer.endWriting();
		m_drawIndex = 0;
	}
	inline virtual void renderTechnique(const float & deltaTime, const std::shared_ptr<Viewport> & viewport, const std::vector<std::pair<int, int>> & perspectives) override {
		if (!m_enabled || !m_shapeQuad->existsYet() || !m_shaderFXAA->existsYet())
			return;

		// Prepare camera index
		if (m_drawIndex >= m_camIndexes.size())
			m_camIndexes.resize(m_drawIndex + 1);
		auto &camBufferIndex = m_camIndexes[m_drawIndex];
		camBufferIndex.beginWriting();
		std::vector<glm::ivec2> camIndices;
		for (auto &[camIndex, layer] : perspectives)
			camIndices.push_back({ camIndex, layer });
		camBufferIndex.write(0, sizeof(glm::ivec2) * camIndices.size(), camIndices.data());
		GLuint instanceCount = perspectives.size();
		m_quadIndirectBuffer.write(sizeof(GLuint), sizeof(GLuint), &instanceCount);
		camBufferIndex.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 3);

		// Apply FXAA effect
		viewport->m_gfxFBOS->bindForWriting("FXAA");
		viewport->m_gfxFBOS->bindForReading("HDR", 0);
		m_shaderFXAA->bind();
		glBindVertexArray(m_shapeQuad->m_vaoID);
		m_quadIndirectBuffer.bindBuffer(GL_DRAW_INDIRECT_BUFFER);
		glDrawArraysIndirect(GL_TRIANGLES, 0);

		// Bind for reading by next effect	
		glBindTextureUnit(0, viewport->m_gfxFBOS->getTexID("FXAA", 0));
		m_drawIndex++;
	}
	

private:
	// Private Attributes
	Engine * m_engine = nullptr;
	Shared_Shader m_shaderFXAA;
	Shared_Primitive m_shapeQuad;
	StaticBuffer m_quadIndirectBuffer;
	std::vector<DynamicBuffer> m_camIndexes;
	int	m_drawIndex = 0;
	std::shared_ptr<bool> m_aliveIndicator = std::make_shared<bool>(true);
};

#endif // FXAA_H