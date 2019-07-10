#pragma once
#ifndef JOIN_REFLECTIONS_H
#define JOIN_REFLECTIONS_H

#include "Modules/Graphics/Common/Graphics_Technique.h"
#include "Assets/Shader.h"
#include "Assets/Texture.h"
#include "Assets/Primitive.h"
#include "Utilities/GL/DynamicBuffer.h"
#include "Utilities/GL/StaticTripleBuffer.h"
#include "Engine.h"


/** A core-rendering technique for writing the final scene reflections back into the lighting. */
class Join_Reflections : public Graphics_Technique {
public:
	// Public (de)Constructors
	/** Virtual Destructor. */
	inline ~Join_Reflections() {
		// Update indicator
		m_aliveIndicator = false;
	}
	/** Constructor. */
	inline Join_Reflections(Engine * engine)
		: m_engine(engine), Graphics_Technique(SECONDARY_LIGHTING) {
		// Asset Loading
		m_shader = Shared_Shader(m_engine, "Effects\\Join Reflections");
		m_brdfMap = Shared_Texture(engine, "brdfLUT.png", GL_TEXTURE_2D, false, false);
		m_shapeQuad = Shared_Primitive(m_engine, "quad");

		// Asset-Finished callbacks
		m_shapeQuad->addCallback(m_aliveIndicator, [&]() mutable {
			const GLuint quadData[4] = { (GLuint)m_shapeQuad->getSize(), 1, 0, 0 }; // count, primCount, first, reserved
			m_quadIndirectBuffer = StaticTripleBuffer(sizeof(GLuint) * 4, quadData);
		});
	}


	// Public Interface Implementations.
	inline virtual void prepareForNextFrame(const float & deltaTime) override {
		for (auto & camIndexBuffer : m_camIndexes)
			camIndexBuffer.endWriting();
		m_quadIndirectBuffer.endWriting();
		m_drawIndex = 0;
	}
	inline virtual void renderTechnique(const float & deltaTime, const std::shared_ptr<Viewport> & viewport, const std::vector<std::pair<int, int>> & perspectives) override {
		if (!m_enabled || !m_shapeQuad->existsYet() || !m_shader->existsYet())
			return;

		// Prepare camera index
		if (m_drawIndex >= m_camIndexes.size())
			m_camIndexes.resize(m_drawIndex + 1);
		auto &camBufferIndex = m_camIndexes[m_drawIndex];
		camBufferIndex.beginWriting();
		m_quadIndirectBuffer.beginWriting();
		std::vector<glm::ivec2> camIndices;
		for (auto &[camIndex, layer] : perspectives)
			camIndices.push_back({ camIndex, layer });
		camBufferIndex.write(0, sizeof(glm::ivec2) * camIndices.size(), camIndices.data());
		GLuint instanceCount = perspectives.size();
		m_quadIndirectBuffer.write(sizeof(GLuint), sizeof(GLuint), &instanceCount);
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
		m_quadIndirectBuffer.bindBuffer(GL_DRAW_INDIRECT_BUFFER);
		glDrawArraysIndirect(GL_TRIANGLES, 0);		
		glDisable(GL_BLEND);
		m_drawIndex++;
	}


private:
	// Private Attributes
	Engine * m_engine = nullptr;
	Shared_Shader m_shader;
	Shared_Texture m_brdfMap;
	Shared_Primitive m_shapeQuad;
	StaticTripleBuffer m_quadIndirectBuffer;
	std::vector<DynamicBuffer> m_camIndexes;
	int	m_drawIndex = 0;
	std::shared_ptr<bool> m_aliveIndicator = std::make_shared<bool>(true);
};

#endif // JOIN_REFLECTIONS_H