#pragma once
#ifndef JOIN_REFLECTIONS_H
#define JOIN_REFLECTIONS_H

#include "Modules/Graphics/Common/Graphics_Technique.h"
#include "Assets/Shader.h"
#include "Assets/Texture.h"
#include "Assets/Auto_Model.h"
#include "Utilities/GL/DynamicBuffer.h"
#include "Utilities/GL/IndirectDraw.h"
#include "Engine.h"


/** A core-rendering technique for writing the final scene reflections back into the lighting. */
class Join_Reflections final : public Graphics_Technique {
public:
	// Public (de)Constructors
	/** Virtual Destructor. */
	inline ~Join_Reflections() {
		// Update indicator
		*m_aliveIndicator = false;
	}
	/** Constructor. */
	inline Join_Reflections(Engine* engine)
		: m_engine(engine), Graphics_Technique(SECONDARY_LIGHTING) {
		// Asset Loading
		m_shader = Shared_Shader(engine, "Effects\\Join Reflections");
		m_brdfMap = Shared_Texture(engine, "brdfLUT.png", GL_TEXTURE_2D, false, false);
		m_shapeQuad = Shared_Auto_Model(engine, "quad");
	}


	// Public Interface Implementations.
	inline virtual void prepareForNextFrame(const float& deltaTime) override final {
		for (auto& [camIndexBuffer, indirectQuad] : m_drawData) {
			camIndexBuffer.endWriting();
			indirectQuad.endWriting();
		}
		m_drawIndex = 0;
	}
	inline virtual void renderTechnique(const float& deltaTime, const std::shared_ptr<Viewport>& viewport, const std::shared_ptr<RH_Volume>& rhVolume, const std::vector<std::pair<int, int>>& perspectives) override final {
		if (!m_enabled || !m_shapeQuad->existsYet() || !m_shader->existsYet())
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
		camBufferIndex.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 3);

		glEnable(GL_BLEND);
		glBlendEquation(GL_FUNC_ADD);
		glBlendFunc(GL_ONE, GL_ONE);
		viewport->m_gfxFBOS->bindForWriting("LIGHTING");
		viewport->m_gfxFBOS->bindForReading("GEOMETRY", 0);
		viewport->m_gfxFBOS->bindForReading("BOUNCE", 4);
		viewport->m_gfxFBOS->bindForReading("REFLECTION", 5);
		m_brdfMap->bind(6);
		m_shader->bind();
		glBindVertexArray(m_shapeQuad->m_vaoID);
		indirectQuad.drawCall();
		glDisable(GL_BLEND);
		Shader::Release();
		m_drawIndex++;
	}


private:
	// Private Attributes
	Engine* m_engine = nullptr;
	Shared_Shader m_shader;
	Shared_Texture m_brdfMap;
	Shared_Auto_Model m_shapeQuad;
	struct DrawData {
		DynamicBuffer camBufferIndex;
		IndirectDraw indirectQuad = IndirectDraw((GLuint)6, 1, 0, GL_DYNAMIC_STORAGE_BIT);
	};
	std::vector<DrawData> m_drawData;
	int	m_drawIndex = 0;
	std::shared_ptr<bool> m_aliveIndicator = std::make_shared<bool>(true);
};

#endif // JOIN_REFLECTIONS_H