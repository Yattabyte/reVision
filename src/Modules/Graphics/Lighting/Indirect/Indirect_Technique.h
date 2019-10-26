#pragma once
#ifndef INDIRECT_TECHNIQUE_H
#define INDIRECT_TECHNIQUE_H

#include "Modules/Graphics/Common/Graphics_Technique.h"
#include "Modules/Graphics/Lighting/Indirect/IndirectData.h"
#include "Modules/Graphics/Lighting/Indirect/IndirectVisibility_System.h"
#include "Modules/Graphics/Lighting/Indirect/IndirectSync_System.h"
#include "Assets/Shader.h"
#include "Assets/Auto_Model.h"
#include "Utilities/GL/StaticMultiBuffer.h"
#include "Utilities/GL/DynamicBuffer.h"
#include "Utilities/GL/IndirectDraw.h"
#include "Engine.h"


/** A core lighting technique responsible for indirect-diffuse lighting. */
class Indirect_Technique final : public Graphics_Technique {
public:
	// Public (de)Constructors
	/** Destructor. */
	inline ~Indirect_Technique() {
		// Update indicator
		*m_aliveIndicator = false;
	}
	/** Constructor. */
	inline Indirect_Technique(Engine* engine, const std::shared_ptr<ShadowData>& shadowData, const std::shared_ptr<Camera>& clientCamera, const std::shared_ptr<std::vector<Camera*>>& cameras)
		: m_engine(engine), m_cameras(cameras), Graphics_Technique(PRIMARY_LIGHTING) {
		m_frameData = std::make_shared<Indirect_Light_Data>();
		m_frameData->clientCamera = clientCamera;
		m_frameData->shadowData = shadowData;

		// Auxiliary Systems
		m_auxilliarySystems.makeSystem<IndirectVisibility_System>(m_frameData);
		m_auxilliarySystems.makeSystem<IndirectSync_System>(m_frameData);

		// Asset Loading
		m_shader_Bounce = Shared_Shader(engine, "Core\\Light\\Bounce");
		m_shader_Recon = Shared_Shader(engine, "Core\\Light\\Reconstruction");
		m_shader_Rebounce = Shared_Shader(engine, "Core\\Light\\Rebounce");
		m_shapeQuad = Shared_Auto_Model(engine, "quad");

		// Noise Texture
		std::uniform_real_distribution<float> randomFloats(0.0, 1.0);
		std::default_random_engine generator;
		glm::vec3 texData[32 * 32 * 32];
		for (int x = 0, total = (32 * 32 * 32); x < total; ++x)
			texData[x] = glm::vec3(randomFloats(generator), randomFloats(generator), randomFloats(generator));
		glCreateTextures(GL_TEXTURE_3D, 1, &m_textureNoise32);
		glTextureImage3DEXT(m_textureNoise32, GL_TEXTURE_3D, 0, GL_RGB16F, 32, 32, 32, 0, GL_RGB, GL_FLOAT, &texData);
		glTextureParameteri(m_textureNoise32, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
		glTextureParameteri(m_textureNoise32, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
		glTextureParameteri(m_textureNoise32, GL_TEXTURE_WRAP_R, GL_MIRRORED_REPEAT);
		glTextureParameteri(m_textureNoise32, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTextureParameteri(m_textureNoise32, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

		// Preferences
		auto& preferences = engine->getPreferenceState();
		preferences.getOrSetValue(PreferenceState::C_RH_BOUNCE_SIZE, m_bounceSize);
		preferences.addCallback(PreferenceState::C_RH_BOUNCE_SIZE, m_aliveIndicator, [&](const float& f) { m_bounceSize = (GLuint)f; });
	}


	// Public Interface Implementations
	inline virtual void prepareForNextFrame(const float& deltaTime) override final {
		m_frameData->lightBuffer.endReading();
		m_drawIndex = 0;
	}
	inline virtual void updateTechnique(const float& deltaTime, ecsWorld& world) override final {
		// Link together the dimensions of view info to that of the viewport vectors
		m_frameData->viewInfo.resize(m_cameras->size());
		world.updateSystems(m_auxilliarySystems, deltaTime);
	}
	inline virtual void renderTechnique(const float& deltaTime, const std::shared_ptr<Viewport>& viewport, const std::shared_ptr<RH_Volume>& rhVolume, const std::vector<std::pair<int, int>>& perspectives) override final {
		// Update light-bounce volume
		if (m_enabled && m_frameData->viewInfo.size() && m_shapeQuad->existsYet() && m_shader_Bounce->existsYet() && m_shader_Recon->existsYet() && m_shader_Rebounce->existsYet()) {
			// Light bounce using client camera
			if (m_drawIndex >= m_drawData.size())
				m_drawData.resize(size_t(m_drawIndex) + 1ull);
			auto& [camBufferIndex, camBufferRebounce, camBufferRecon, visLights, indirectBounce, indirectQuad, indirectQuadRecon] = m_drawData[m_drawIndex];

			// Accumulate all visibility info for the cameras passed in
			std::vector<glm::ivec2> camIndicesGen, camIndiciesRebounce, camIndiciesRecon;
			std::vector<GLint> lightIndices;
			for (auto& [camIndex, layer] : perspectives) {
				const std::vector<glm::ivec2> tempIndices(m_frameData->viewInfo[camIndex].lightIndices.size(), { camIndex, layer });
				camIndicesGen.insert(camIndicesGen.end(), tempIndices.begin(), tempIndices.end());
				lightIndices.insert(lightIndices.end(), m_frameData->viewInfo[camIndex].lightIndices.begin(), m_frameData->viewInfo[camIndex].lightIndices.end());
				const std::vector<glm::ivec2> tempIndices2(m_bounceSize, { camIndex, layer });
				camIndiciesRebounce.insert(camIndiciesRebounce.end(), tempIndices2.begin(), tempIndices2.end());
				camIndiciesRecon.push_back({ camIndex, layer });
			}
			const auto shadowCount = camIndicesGen.size();

			if (lightIndices.size()) {
				updateDrawParams(camBufferIndex, camBufferRebounce, camBufferRecon, visLights, indirectBounce, indirectQuad, indirectQuadRecon, camIndicesGen, camIndiciesRebounce, camIndiciesRecon, lightIndices, shadowCount, perspectives);
				fillBounceVolume(shadowCount, rhVolume);
				rebounceVolume(rhVolume, camBufferRebounce, indirectQuad);
				reconstructVolume(viewport, rhVolume, camBufferRecon, indirectQuadRecon);
				camBufferIndex.endReading();
				camBufferRebounce.endReading();
				camBufferRecon.endReading();
				visLights.endReading();
				indirectBounce.endReading();
				indirectQuad.endReading();
				indirectQuadRecon.endReading();
				Shader::Release();

				m_drawIndex++;
			}
		}
	}


private:
	// Private Methods
	/***/
	inline void updateDrawParams(DynamicBuffer<>& camBufferIndex, DynamicBuffer<>& camBufferRebounce, DynamicBuffer<>& camBufferRecon, DynamicBuffer<>& visLights, StaticMultiBuffer<>& indirectBounce, IndirectDraw<>& indirectQuad, IndirectDraw<>& indirectQuadRecon, std::vector<glm::ivec2>& camIndicesGen, std::vector<glm::ivec2>& camIndiciesRebounce, std::vector<glm::ivec2>& camIndiciesRecon, std::vector<int>& lightIndices, const size_t& shadowCount, const std::vector<std::pair<int, int>>& perspectives) {
		// Write accumulated data
		camBufferIndex.beginWriting();
		camBufferRebounce.beginWriting();
		camBufferRecon.beginWriting();
		visLights.beginWriting();
		indirectBounce.beginWriting();
		indirectQuad.beginWriting();
		indirectQuadRecon.beginWriting();
		camBufferIndex.write(0, sizeof(glm::ivec2) * camIndicesGen.size(), camIndicesGen.data());
		camBufferRebounce.write(0, sizeof(glm::ivec2) * camIndiciesRebounce.size(), camIndiciesRebounce.data());
		camBufferRecon.write(0, sizeof(glm::ivec2) * camIndiciesRecon.size(), camIndiciesRecon.data());
		visLights.write(0, sizeof(GLuint) * lightIndices.size(), lightIndices.data());
		const GLuint dataBounce[] = { (GLuint)m_shapeQuad->getSize(), (GLuint)(shadowCount * m_bounceSize), 0, 0 };
		indirectBounce.write(0, sizeof(GLuint) * 4, &dataBounce);
		indirectQuad.setPrimitiveCount(m_bounceSize);
		indirectQuadRecon.setPrimitiveCount((GLuint)perspectives.size());
		camBufferIndex.endWriting();
		camBufferRebounce.endWriting();
		camBufferRecon.endWriting();
		visLights.endWriting();
		indirectBounce.endWriting();
		indirectQuad.endWriting();
		indirectQuadRecon.endWriting();
	}
	/***/
	inline void fillBounceVolume(const size_t& shadowCount, const std::shared_ptr<RH_Volume>& rhVolume) {
		// Prepare rendering state
		glBlendEquationSeparatei(0, GL_MIN, GL_MIN);
		glBindVertexArray(m_shapeQuad->m_vaoID);
		m_shader_Bounce->setUniform(0, (GLint)(shadowCount));
		m_shader_Bounce->setUniform(1, rhVolume->m_max);
		m_shader_Bounce->setUniform(2, rhVolume->m_min);
		m_shader_Bounce->setUniform(4, rhVolume->m_resolution);
		m_shader_Bounce->setUniform(6, rhVolume->m_unitSize);

		glViewport(0, 0, (GLsizei)rhVolume->m_resolution, (GLsizei)rhVolume->m_resolution);
		m_shader_Bounce->bind();
		rhVolume->writePrimary();
		glBindTextureUnit(0, m_frameData->shadowData->shadowFBO.m_texNormal);
		glBindTextureUnit(1, m_frameData->shadowData->shadowFBO.m_texColor);
		glBindTextureUnit(2, m_frameData->shadowData->shadowFBO.m_texDepth);
		glBindTextureUnit(4, m_textureNoise32);
		m_drawData[m_drawIndex].bufferCamIndex.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 3);
		m_drawData[m_drawIndex].visLights.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 4);
		m_frameData->lightBuffer.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 8);
		m_drawData[m_drawIndex].indirectBounce.bindBuffer(GL_DRAW_INDIRECT_BUFFER);
		glDrawArraysIndirect(GL_TRIANGLES, 0);
	}
	/***/
	inline void rebounceVolume(const std::shared_ptr<RH_Volume>& rhVolume, DynamicBuffer<>& camBufferRebounce, IndirectDraw<>& indirectQuad) {
		// Bind common data
		glDepthMask(GL_TRUE);
		glBlendEquation(GL_FUNC_ADD);
		glBlendFunc(GL_ONE, GL_ZERO);
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_BLEND);
		m_shader_Rebounce->setUniform(1, rhVolume->m_max);
		m_shader_Rebounce->setUniform(2, rhVolume->m_min);
		m_shader_Rebounce->setUniform(4, rhVolume->m_resolution);
		m_shader_Rebounce->setUniform(5, rhVolume->m_unitSize);
		m_shader_Recon->setUniform(1, rhVolume->m_max);
		m_shader_Recon->setUniform(2, rhVolume->m_min);
		m_shader_Recon->setUniform(3, rhVolume->m_resolution);
		glBindVertexArray(m_shapeQuad->m_vaoID);

		// Bounce light a second time
		m_shader_Rebounce->bind();
		rhVolume->readPrimary(0);
		rhVolume->writeSecondary();
		glViewport(0, 0, (GLsizei)rhVolume->m_resolution, (GLsizei)rhVolume->m_resolution);
		camBufferRebounce.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 3);
		indirectQuad.drawCall();
	}
	/***/
	inline void reconstructVolume(const std::shared_ptr<Viewport>& viewport, const std::shared_ptr<RH_Volume>& rhVolume, DynamicBuffer<>& camBufferRecon, IndirectDraw<>& indirectQuadRecon) {
		// Reconstruct indirect radiance
		glViewport(0, 0, GLsizei(viewport->m_dimensions.x), GLsizei(viewport->m_dimensions.y));
		m_shader_Recon->bind();
		viewport->m_gfxFBOS->bindForReading("GEOMETRY", 0);
		rhVolume->readSecondary(4);
		viewport->m_gfxFBOS->bindForWriting("BOUNCE");
		camBufferRecon.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 3);
		indirectQuadRecon.drawCall();
		glEnable(GL_DEPTH_TEST);
	}


	// Private Attributes
	Engine* m_engine = nullptr;
	Shared_Shader m_shader_Bounce, m_shader_Recon, m_shader_Rebounce;
	Shared_Auto_Model m_shapeQuad;
	GLuint m_textureNoise32 = 0;
	GLuint m_bounceSize = 16u;
	struct DrawData {
		DynamicBuffer<> bufferCamIndex, bufferCamRebounce, bufferCamRecon, visLights;
		StaticMultiBuffer<> indirectBounce = StaticMultiBuffer(sizeof(GLuint) * 4);
		IndirectDraw<> indirectQuad = IndirectDraw((GLuint)6, 1, 0, GL_DYNAMIC_STORAGE_BIT), indirectQuadRecon = IndirectDraw((GLuint)6, 1, 0, GL_DYNAMIC_STORAGE_BIT);
	};
	int m_drawIndex = 0;
	std::vector<DrawData> m_drawData;
	ecsSystemList m_auxilliarySystems;

	// Shared Attributes
	std::shared_ptr<Indirect_Light_Data> m_frameData;
	std::shared_ptr<std::vector<Camera*>> m_cameras;
	std::shared_ptr<bool> m_aliveIndicator = std::make_shared<bool>(true);
};

#endif // INDIRECT_TECHNIQUE_H