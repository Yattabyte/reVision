#pragma once
#ifndef RADIANCE_HINTS_H
#define RADIANCE_HINTS_H

#include "Modules/Graphics/Common/Graphics_Technique.h"
#include "Modules/Graphics/Common/Graphics_Framebuffers.h"
#include "Modules/Graphics/Common/Camera.h"
#include "Modules/Graphics/Common/RH_Volume.h"
#include "Assets/Shader.h"
#include "Assets/Texture.h"
#include "Assets/Auto_Model.h"
#include "Utilities/GL/DynamicBuffer.h"
#include "Utilities/GL/IndirectDraw.h"
#include "Engine.h"


/** A core-rendering technique for approximating indirect diffuse lighting (irradiant light, global illumination, etc) */
class Radiance_Hints final : public Graphics_Technique {
public:
	// Public (de)Constructors
	/** Virtual Destructor. */
	inline ~Radiance_Hints() {
		// Update indicator
		*m_aliveIndicator = false;
	}
	/** Constructor. */
	inline explicit Radiance_Hints(Engine* engine)
		: m_engine(engine), Graphics_Technique(SECONDARY_LIGHTING) {
		// Asset Loading
		m_shaderRecon = Shared_Shader(engine, "Effects\\RH Reconstruction");
		m_shaderRebounce = Shared_Shader(engine, "Effects\\RH Rebounce");
		m_shapeQuad = Shared_Auto_Model(engine, "quad");

		// Preferences
		auto& preferences = m_engine->getPreferenceState();
		preferences.getOrSetValue(PreferenceState::C_RH_BOUNCE_SIZE, m_bounceSize);
		preferences.addCallback(PreferenceState::C_RH_BOUNCE_SIZE, m_aliveIndicator, [&](const float& f) {
			m_bounceSize = (GLuint)f;
			});
	}


	// Public Interface Implementations.
	inline virtual void prepareForNextFrame(const float& deltaTime) override final {
		m_drawIndex = 0;
	}
	inline virtual void renderTechnique(const float& deltaTime, const std::shared_ptr<Viewport>& viewport, const std::shared_ptr<RH_Volume>& rhVolume, const std::vector<std::pair<int, int>>& perspectives) override final {
		if (!m_enabled || !m_shapeQuad->existsYet() || !m_shaderRecon->existsYet() || !m_shaderRebounce->existsYet())
			return;

		// Prepare camera index
		if (m_drawIndex >= m_drawData.size())
			m_drawData.resize(size_t(m_drawIndex) + 1ull);
		auto& [camBufferRebounce, camBufferRecon, indirectQuad, indirectQuadRecon] = m_drawData[m_drawIndex];
		camBufferRebounce.beginWriting();
		camBufferRecon.beginWriting();
		indirectQuad.beginWriting();
		indirectQuadRecon.beginWriting();
		std::vector<glm::ivec2> camIndiciesRebounce, camIndiciesRecon;
		for (auto& [camIndex, layer] : perspectives) {
			const std::vector<glm::ivec2> tempIndices(m_bounceSize, { camIndex, layer });
			camIndiciesRebounce.insert(camIndiciesRebounce.end(), tempIndices.begin(), tempIndices.end());
			camIndiciesRecon.push_back({ camIndex, layer });
		}
		camBufferRebounce.write(0, sizeof(glm::ivec2) * camIndiciesRebounce.size(), camIndiciesRebounce.data());
		camBufferRecon.write(0, sizeof(glm::ivec2) * camIndiciesRecon.size(), camIndiciesRecon.data());
		indirectQuad.setPrimitiveCount(m_bounceSize);
		indirectQuadRecon.setPrimitiveCount((GLuint)perspectives.size());
		camBufferRebounce.endWriting();
		camBufferRecon.endWriting();
		indirectQuad.endWriting();
		indirectQuadRecon.endWriting();

		// Bind common data
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_BLEND);
		m_shaderRebounce->setUniform(1, rhVolume->m_max);
		m_shaderRebounce->setUniform(2, rhVolume->m_min);
		m_shaderRebounce->setUniform(4, rhVolume->m_resolution);
		m_shaderRebounce->setUniform(5, rhVolume->m_unitSize);
		m_shaderRecon->setUniform(1, rhVolume->m_max);
		m_shaderRecon->setUniform(2, rhVolume->m_min);
		m_shaderRecon->setUniform(3, rhVolume->m_resolution);
		glBindVertexArray(m_shapeQuad->m_vaoID);

		// Bounce light a second time
		m_shaderRebounce->bind();
		rhVolume->readPrimary(0);
		rhVolume->writeSecondary();
		glViewport(0, 0, (GLsizei)rhVolume->m_resolution, (GLsizei)rhVolume->m_resolution);
		camBufferRebounce.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 3);
		indirectQuad.drawCall();
		indirectQuad.endReading();
		camBufferRebounce.endReading();

		// Reconstruct indirect radiance
		glViewport(0, 0, GLsizei(viewport->m_dimensions.x), GLsizei(viewport->m_dimensions.y));
		m_shaderRecon->bind();
		viewport->m_gfxFBOS->bindForReading("GEOMETRY", 0);
		rhVolume->readSecondary(4);
		viewport->m_gfxFBOS->bindForWriting("BOUNCE");
		camBufferRecon.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 3);
		indirectQuadRecon.drawCall();
		indirectQuadRecon.endReading();
		camBufferRecon.endReading();

		Shader::Release();
		m_drawIndex++;
	}


private:
	// Private Attributes
	Engine* m_engine = nullptr;
	Shared_Shader m_shaderRecon, m_shaderRebounce;
	Shared_Auto_Model m_shapeQuad;
	GLuint m_bounceSize = 16;
	struct DrawData {
		DynamicBuffer<> camBufferRebounce, camBufferRecon;
		IndirectDraw<> indirectQuad = IndirectDraw((GLuint)6, 1, 0, GL_DYNAMIC_STORAGE_BIT), indirectQuadRecon = IndirectDraw((GLuint)6, 1, 0, GL_DYNAMIC_STORAGE_BIT);
	};
	std::vector<DrawData> m_drawData;
	int	m_drawIndex = 0;
	std::shared_ptr<bool> m_aliveIndicator = std::make_shared<bool>(true);
};

#endif // RADIANCE_HINTS_H