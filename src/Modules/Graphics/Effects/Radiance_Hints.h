#pragma once
#ifndef RADIANCE_HINTS_H
#define RADIANCE_HINTS_H

#include "Modules/Graphics/Common/Graphics_Technique.h"
#include "Modules/Graphics/Common/Graphics_Framebuffers.h"
#include "Modules/Graphics/Common/Camera.h"
#include "Modules/Graphics/Common/RH_Volume.h"
#include "Assets/Shader.h"
#include "Assets/Texture.h"
#include "Assets/Primitive.h"
#include "Utilities/GL/DynamicBuffer.h"
#include "Utilities/GL/StaticTripleBuffer.h"
#include "Engine.h"


/** A core-rendering technique for approximating indirect diffuse lighting (irradiant light, global illumination, etc) */
class Radiance_Hints : public Graphics_Technique {
public:
	// Public (de)Constructors
	/** Virtual Destructor. */
	inline ~Radiance_Hints() {
		// Update indicator
		m_aliveIndicator = false;
	}
	/** Constructor. */
	inline Radiance_Hints(Engine * engine, const std::shared_ptr<RH_Volume> & rhVolume)
		: m_engine(engine), m_rhVolume(rhVolume), Graphics_Technique(SECONDARY_LIGHTING) {
		// Asset Loading
		m_shaderRecon = Shared_Shader(m_engine, "Effects\\RH Reconstruction");
		m_shaderRebounce = Shared_Shader(m_engine, "Effects\\RH Rebounce");
		m_shapeQuad = Shared_Primitive(m_engine, "quad");

		// Preferences
		auto & preferences = m_engine->getPreferenceState();
		preferences.getOrSetValue(PreferenceState::C_RH_BOUNCE_SIZE, m_bounceSize);
		preferences.addCallback(PreferenceState::C_RH_BOUNCE_SIZE, m_aliveIndicator, [&](const float &f) { 
			m_bounceSize = (GLuint)f; 
		});
	}


	// Public Interface Implementations.
	inline virtual void prepareForNextFrame(const float & deltaTime) override {
		for (auto &[camBufferRebounce, camBufferRecon, quadIndirectBuffer, quadIndirectBufferRecon] : m_drawData) {
			camBufferRebounce.endWriting();
			camBufferRecon.endWriting();
			quadIndirectBuffer.endWriting();
			quadIndirectBufferRecon.endWriting();
		}
		m_drawIndex = 0;
	}
	inline virtual void renderTechnique(const float & deltaTime, const std::shared_ptr<Viewport> & viewport, const std::vector<std::pair<int, int>> & perspectives) override {
		if (!m_enabled || !m_shapeQuad->existsYet() || !m_shaderRecon->existsYet() || !m_shaderRebounce->existsYet())
			return;

		// Prepare camera index
		if (m_drawIndex >= m_drawData.size())
			m_drawData.resize(m_drawIndex + 1);
		auto &[camBufferRebounce, camBufferRecon, quadIndirectBuffer, quadIndirectBufferRecon] = m_drawData[m_drawIndex];
		camBufferRebounce.beginWriting();
		camBufferRecon.beginWriting();
		quadIndirectBuffer.beginWriting();
		quadIndirectBufferRecon.beginWriting();
		std::vector<glm::ivec2> camIndiciesRebounce, camIndiciesRecon;	
		for (auto &[camIndex, layer] : perspectives) {
			const std::vector<glm::ivec2> tempIndices(m_bounceSize, { camIndex, layer });
			camIndiciesRebounce.insert(camIndiciesRebounce.end(), tempIndices.begin(), tempIndices.end());
			camIndiciesRecon.push_back({ camIndex, layer });
		}
		camBufferRebounce.write(0, sizeof(glm::ivec2) * camIndiciesRebounce.size(), camIndiciesRebounce.data());
		camBufferRecon.write(0, sizeof(glm::ivec2) * camIndiciesRecon.size(), camIndiciesRecon.data());
		const auto bounceCount = m_bounceSize;
		const auto reconCount = (GLuint)perspectives.size();
		quadIndirectBuffer.write(sizeof(GLuint), sizeof(GLuint), &bounceCount);
		quadIndirectBufferRecon.write(sizeof(GLuint), sizeof(GLuint), &reconCount);
		
		// Bind common data
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_BLEND);
		m_shaderRebounce->setUniform(1, m_rhVolume->m_max);
		m_shaderRebounce->setUniform(2, m_rhVolume->m_min);
		m_shaderRebounce->setUniform(4, m_rhVolume->m_resolution);
		m_shaderRebounce->setUniform(5, m_rhVolume->m_unitSize);
		m_shaderRecon->setUniform(1, m_rhVolume->m_max);
		m_shaderRecon->setUniform(2, m_rhVolume->m_min);
		m_shaderRecon->setUniform(3, m_rhVolume->m_resolution);
		glBindVertexArray(m_shapeQuad->m_vaoID);

		// Bounce light a second time
		m_shaderRebounce->bind();
		m_rhVolume->readPrimary(0);
		m_rhVolume->writeSecondary();
		glViewport(0, 0, (GLsizei)m_rhVolume->m_resolution, (GLsizei)m_rhVolume->m_resolution);
		quadIndirectBuffer.bindBuffer(GL_DRAW_INDIRECT_BUFFER);
		camBufferRebounce.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 3);
		glDrawArraysIndirect(GL_TRIANGLES, 0);

		// Reconstruct indirect radiance
		glViewport(0, 0, GLsizei(viewport->m_dimensions.x), GLsizei(viewport->m_dimensions.y));
		m_shaderRecon->bind();
		viewport->m_gfxFBOS->bindForReading("GEOMETRY", 0);
		m_rhVolume->readSecondary(4);
		viewport->m_gfxFBOS->bindForWriting("BOUNCE");
		quadIndirectBufferRecon.bindBuffer(GL_DRAW_INDIRECT_BUFFER);
		camBufferRecon.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 3);
		glDrawArraysIndirect(GL_TRIANGLES, 0);		
		m_drawIndex++;
	}


private:
	// Private Attributes
	Engine * m_engine = nullptr;
	std::shared_ptr<RH_Volume> m_rhVolume;
	Shared_Shader m_shaderRecon, m_shaderRebounce;
	Shared_Primitive m_shapeQuad;
	GLuint m_bounceSize = 16;
	struct DrawData {
		DynamicBuffer camBufferRebounce, camBufferRecon;
		constexpr static GLuint quadData[4] = { (GLuint)6, 1, 0, 0 };
		StaticTripleBuffer quadIndirectBuffer = StaticTripleBuffer(sizeof(GLuint) * 4, quadData), quadIndirectBufferRecon = StaticTripleBuffer(sizeof(GLuint) * 4, quadData);
	};
	std::vector<DrawData> m_drawData;
	int	m_drawIndex = 0;
	std::shared_ptr<bool> m_aliveIndicator = std::make_shared<bool>(true);
};

#endif // RADIANCE_HINTS_H