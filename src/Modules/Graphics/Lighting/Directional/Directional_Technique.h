#pragma once
#ifndef DIRECTIONAL_TECHNIQUE_H
#define DIRECTIONAL_TECHNIQUE_H

#include "Modules/Graphics/Common/Graphics_Technique.h"
#include "Modules/Graphics/Lighting/Directional/DirectionalData.h"
#include "Modules/Graphics/Lighting/Directional/DirectionalVisibility_System.h"
#include "Modules/Graphics/Lighting/Directional/DirectionalSync_System.h"
#include "Modules/ECS/ecsSystem.h"
#include "Assets/Shader.h"
#include "Assets/Auto_Model.h"
#include "Utilities/GL/StaticTripleBuffer.h"
#include "Engine.h"
#include <random>


/** A core lighting technique responsible for all directional lights. */
class Directional_Technique final : public Graphics_Technique {
public:
	// Public (de)Constructors
	/** Destructor. */
	inline ~Directional_Technique() {
		// Update indicator
		*m_aliveIndicator = false;
	}
	/** Constructor. */
	inline Directional_Technique(Engine* engine, const std::shared_ptr<ShadowData>& shadowData, const std::shared_ptr<Camera>& clientCamera, const std::shared_ptr<std::vector<Camera*>>& cameras, ecsSystemList& auxilliarySystems)
		: m_engine(engine), m_cameras(cameras), Graphics_Technique(PRIMARY_LIGHTING) {
		// Auxilliary Systems
		m_frameData = std::make_shared<DirectionalData>();
		m_frameData->clientCamera = clientCamera;
		m_frameData->shadowData = shadowData;
		auxilliarySystems.makeSystem<DirectionalVisibility_System>(m_frameData);
		auxilliarySystems.makeSystem<DirectionalSync_System>(m_frameData);

		// Asset Loading
		m_shader_Lighting = Shared_Shader(engine, "Core\\Directional\\Light");
		m_shader_Bounce = Shared_Shader(engine, "Core\\Directional\\Bounce");
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
		m_frameData->lightBuffer.endWriting();
		for (auto& drawBuffer : m_drawData) {
			drawBuffer.bufferCamIndex.endWriting();
			drawBuffer.visLights.endWriting();
			drawBuffer.indirectShape.endWriting();
		}
		for (auto& bounceBuffer : m_drawBounceData) {
			bounceBuffer.bufferCamIndex.endWriting();
			bounceBuffer.visLights.endWriting();
			bounceBuffer.indirectBounce.endWriting();
		}
		m_drawIndex = 0;
		m_bounceIndex = 0;
		//clear();
	}
	inline virtual void updateTechnique(const float& deltaTime) override final {
		// Link together the dimensions of view info to that of the viewport vectors
		m_frameData->viewInfo.resize(m_cameras->size());
	}
	inline virtual void renderTechnique(const float& deltaTime, const std::shared_ptr<Viewport>& viewport, const std::shared_ptr<RH_Volume>& rhVolume, const std::vector<std::pair<int, int>>& perspectives) override final {
		// Exit Early
		if (m_enabled && m_frameData->viewInfo.size() && m_shapeQuad->existsYet() && m_shader_Lighting->existsYet() && m_shader_Bounce->existsYet()) {
			/*// Light Bounce
			{
				// Find client camera
				size_t visibilityIndex = 0;
				bool found = false;
				for (size_t x = 0; x < m_cameras->size(); ++x)
					if (m_cameras->at(x) == m_frameData->clientCamera.get()) {
						visibilityIndex = x;
						found = true;
						break;
					}
				if (found && visibilityIndex < m_frameData->viewInfo.size() && m_frameData->viewInfo[visibilityIndex].visShadowCount) {
					// Light bounce using client camera
					if (m_bounceIndex >= m_drawBounceData.size())
						m_drawBounceData.resize(size_t(m_drawIndex) + 1ull);

					auto& bounceBuffer = m_drawBounceData[m_bounceIndex];
					auto& camBufferIndex = bounceBuffer.bufferCamIndex;
					auto& lightBufferIndex = bounceBuffer.visLights;
					camBufferIndex.beginWriting();
					lightBufferIndex.beginWriting();
					bounceBuffer.indirectBounce.beginWriting();

					// Accumulate all visibility info for the cameras passed in
					std::vector<glm::ivec2> camIndices(m_frameData->viewInfo[visibilityIndex].lightIndices.size(), { visibilityIndex, 0 });
					std::vector<GLint> lightIndices(m_frameData->viewInfo[visibilityIndex].lightIndices.begin(), m_frameData->viewInfo[visibilityIndex].lightIndices.end());
					const auto shadowCount = m_frameData->viewInfo[visibilityIndex].visShadowCount;

					if (lightIndices.size()) {
						// Write accumulated data
						camBufferIndex.write(0, sizeof(glm::ivec2) * camIndices.size(), camIndices.data());
						lightBufferIndex.write(0, sizeof(GLuint) * lightIndices.size(), lightIndices.data());
						const GLuint dataBounce[] = { (GLuint)m_shapeQuad->getSize(), (GLuint)(shadowCount * m_bounceSize),0,0 };
						bounceBuffer.indirectBounce.write(0, sizeof(GLuint) * 4, &dataBounce);

						// Update light bounce
						renderBounce(deltaTime, (int)visibilityIndex, rhVolume);

						m_bounceIndex++;
					}
				}
			}*/
			// Render Light
			{
				if (m_drawIndex >= m_drawData.size())
					m_drawData.resize(size_t(m_drawIndex) + 1ull);
				auto& drawBuffer = m_drawData[m_drawIndex];
				auto& camBufferIndex = drawBuffer.bufferCamIndex;
				auto& lightBufferIndex = drawBuffer.visLights;
				camBufferIndex.beginWriting();
				lightBufferIndex.beginWriting();
				drawBuffer.indirectShape.beginWriting();

				// Accumulate all visibility info for the cameras passed in
				std::vector<glm::ivec2> camIndices;
				std::vector<GLint> lightIndices;
				for (auto& [camIndex, layer] : perspectives) {
					const std::vector<glm::ivec2> tempIndices(m_frameData->viewInfo[camIndex].lightIndices.size(), { camIndex, layer });
					camIndices.insert(camIndices.end(), tempIndices.begin(), tempIndices.end());
					lightIndices.insert(lightIndices.end(), m_frameData->viewInfo[camIndex].lightIndices.begin(), m_frameData->viewInfo[camIndex].lightIndices.end());
				}

				if (lightIndices.size()) {
					// Write accumulated data
					camBufferIndex.write(0, sizeof(glm::ivec2) * camIndices.size(), camIndices.data());
					lightBufferIndex.write(0, sizeof(GLuint) * lightIndices.size(), lightIndices.data());
					const GLuint dataShape[] = { (GLuint)m_shapeQuad->getSize(), (GLuint)lightIndices.size(), 0,0 };
					drawBuffer.indirectShape.write(0, sizeof(GLuint) * 4, &dataShape);

					// Render lights
					renderLights(deltaTime, viewport);

					m_drawIndex++;
				}
			}
		}
	}


private:
	// Private Methods
	/** Render all the lights.
	@param	deltaTime	the amount of time passed since last frame.
	@param	viewport	the viewport to render from. */
	inline void renderLights(const float& deltaTime, const std::shared_ptr<Viewport>& viewport) {
		// Prepare rendering state
		glEnable(GL_BLEND);
		glBlendEquation(GL_FUNC_ADD);
		glBlendFunc(GL_ONE, GL_ONE);
		glDisable(GL_DEPTH_TEST);
		glDepthMask(GL_FALSE);

		m_shader_Lighting->bind();									// Shader (directional)
		viewport->m_gfxFBOS->bindForWriting("LIGHTING");			// Ensure writing to lighting FBO
		viewport->m_gfxFBOS->bindForReading("GEOMETRY", 0);			// Read from Geometry FBO
		glBindTextureUnit(4, m_frameData->shadowData->shadowFBO.m_texDepth);			// Shadow map (depth texture)
		m_drawData[m_drawIndex].bufferCamIndex.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 3);
		m_drawData[m_drawIndex].visLights.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 4);	// SSBO visible light indices
		m_frameData->lightBuffer.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 8);
		m_drawData[m_drawIndex].indirectShape.bindBuffer(GL_DRAW_INDIRECT_BUFFER);		// Draw call buffer
		glBindVertexArray(m_shapeQuad->m_vaoID);					// Quad VAO
		glDrawArraysIndirect(GL_TRIANGLES, 0);						// Now draw
	}
	/** Render light bounces.
	@param	deltaTime	the amount of time passed since last frame.
	@param	viewport	the viewport to render from. */
	inline void renderBounce(const float& deltaTime, const int& viewingIndex, const std::shared_ptr<RH_Volume>& rhVolume) {
		// Prepare rendering state
		glBlendEquationSeparatei(0, GL_MIN, GL_MIN);
		glBindVertexArray(m_shapeQuad->m_vaoID);
		m_shader_Bounce->setUniform(0, (GLint)(m_frameData->viewInfo.at(viewingIndex).visShadowCount));
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
		m_drawBounceData[m_bounceIndex].bufferCamIndex.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 3);
		m_drawBounceData[m_bounceIndex].visLights.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 4);
		m_frameData->lightBuffer.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 8);
		m_drawBounceData[m_bounceIndex].indirectBounce.bindBuffer(GL_DRAW_INDIRECT_BUFFER);
		glDrawArraysIndirect(GL_TRIANGLES, 0);

		glDepthMask(GL_TRUE);
		glEnable(GL_DEPTH_TEST);
		glBlendEquation(GL_FUNC_ADD);
		glBlendFunc(GL_ONE, GL_ZERO);
		glDisable(GL_BLEND);
	}
	/** Clear out the lights and shadows queued up for rendering. */
	inline void clear() {
		m_frameData->viewInfo.clear();
		m_drawData.clear();
		m_drawBounceData.clear();
		m_drawIndex = 0;
		m_bounceIndex = 0;
	}


	// Private Attributes
	Engine* m_engine = nullptr;
	std::shared_ptr<bool> m_aliveIndicator = std::make_shared<bool>(true);
	Shared_Shader m_shader_Lighting, m_shader_Bounce;
	Shared_Auto_Model m_shapeQuad;
	GLuint m_textureNoise32 = 0;
	GLuint m_bounceSize = 16u;
	struct DrawData {
		DynamicBuffer bufferCamIndex;
		DynamicBuffer visLights;
		StaticTripleBuffer indirectShape = StaticTripleBuffer(sizeof(GLuint) * 4);
	};
	int m_drawIndex = 0;
	std::vector<DrawData> m_drawData;
	struct DrawBounceData {
		DynamicBuffer bufferCamIndex;
		DynamicBuffer visLights;
		StaticTripleBuffer indirectBounce = StaticTripleBuffer(sizeof(GLuint) * 4);
	};
	int m_bounceIndex = 0;
	std::vector<DrawBounceData> m_drawBounceData;

	// Shared Attributes
	std::shared_ptr<DirectionalData> m_frameData;
	std::shared_ptr<std::vector<Camera*>> m_cameras;
};

#endif // DIRECTIONAL_TECHNIQUE_H