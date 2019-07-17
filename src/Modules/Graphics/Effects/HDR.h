#pragma once
#ifndef HDR_H
#define HDR_H

#include "Modules/Graphics/Common/Graphics_Technique.h"
#include "Assets/Shader.h"
#include "Assets/Primitive.h"
#include "Utilities/GL/DynamicBuffer.h"
#include "Utilities/GL/StaticTripleBuffer.h"
#include "Engine.h"


/** A post-processing technique for tone-mapping and gamma correcting the final lighting product. */
class HDR : public Graphics_Technique {
public:
	// Public (de)Constructors
	/** Virtual Destructor. */
	inline ~HDR() {
		// Update indicator
		m_aliveIndicator = false;
	}
	/** Constructor. */
	inline HDR(Engine * engine)
		: m_engine(engine), Graphics_Technique(POST_PROCESSING) {
		// Asset Loading
		m_shaderHDR = Shared_Shader(m_engine, "Effects\\HDR");
		m_shapeQuad = Shared_Primitive(engine, "quad");

		// Preferences
		auto & preferences = m_engine->getPreferenceState();
		preferences.getOrSetValue(PreferenceState::C_GAMMA, m_gamma);
		preferences.addCallback(PreferenceState::C_GAMMA, m_aliveIndicator, [&](const float &f) { m_gamma = f; });
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
		if (!m_enabled || !m_shapeQuad->existsYet() || !m_shaderHDR->existsYet())
			return;
		
		// Prepare camera index
		if (m_drawIndex >= m_drawData.size())
			m_drawData.resize(m_drawIndex + 1);
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

		// Write HDR effect to own framebuffer
		viewport->m_gfxFBOS->bindForWriting("HDR");
		viewport->m_gfxFBOS->bindForReading("LIGHTING", 0);
		m_shaderHDR->bind();
		m_shaderHDR->setUniform(0, 1.0f);
		m_shaderHDR->setUniform(1, m_gamma);
		// Use the currently bound framebuffer from the prior effect
		glBindVertexArray(m_shapeQuad->m_vaoID);
		quadIndirectBuffer.bindBuffer(GL_DRAW_INDIRECT_BUFFER);
		glDrawArraysIndirect(GL_TRIANGLES, 0);	
		m_drawIndex++;
	}


private:
	// Private Attributes
	Engine * m_engine = nullptr;
	float m_gamma = 1.0f;
	Shared_Shader m_shaderHDR;
	Shared_Primitive m_shapeQuad;
	struct DrawData {
		DynamicBuffer camBufferIndex;
		constexpr static GLuint quadData[4] = { (GLuint)6, 1, 0, 0 };
		StaticTripleBuffer quadIndirectBuffer = StaticTripleBuffer(sizeof(GLuint) * 4, quadData);
	};
	std::vector<DrawData> m_drawData;
	int	m_drawIndex = 0;
	std::shared_ptr<bool> m_aliveIndicator = std::make_shared<bool>(true);
};

#endif // HDR_H