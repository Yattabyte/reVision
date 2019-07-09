#pragma once
#ifndef DIRECTIONAL_TECHNIQUE_H
#define DIRECTIONAL_TECHNIQUE_H

#include "Modules/Graphics/Common/Graphics_Technique.h"
#include "Modules/Graphics/Lighting/components.h"
#include "Modules/Graphics/Lighting/Directional/DirectionalData.h"
#include "Modules/Graphics/Lighting/Directional/DirectionalVisibility_System.h"
#include "Modules/Graphics/Lighting/Directional/DirectionalSync_System.h"
#include "Modules/World/ECS/components.h"
#include "Modules/World/ECS/ecsSystem.h"
#include "Assets/Shader.h"
#include "Assets/Primitive.h"
#include "Utilities/GL/StaticTripleBuffer.h"
#include "Engine.h"
#include <random>


/** A core lighting technique responsible for all directional lights. */
class Directional_Technique : public Graphics_Technique {
public:
	// Public (de)Constructors
	/** Destructor. */
	inline ~Directional_Technique() {
		// Update indicator
		m_aliveIndicator = false;
	}
	/** Constructor. */
	inline Directional_Technique(Engine * engine, const std::shared_ptr<ShadowData> & shadowData, const std::shared_ptr<RH_Volume> & rhVolume, const std::shared_ptr<CameraBuffer> & clientCamera, const std::shared_ptr<std::vector<CameraBuffer::CamStruct*>> & cameras, ECSSystemList & auxilliarySystems)
		: m_engine(engine), m_rhVolume(rhVolume), m_cameras(cameras), Graphics_Technique(PRIMARY_LIGHTING) {
		// Auxilliary Systems
		m_frameData = std::make_shared<DirectionalData>();
		m_frameData->clientCamera = clientCamera;
		m_frameData->shadowData = shadowData;
		auxilliarySystems.addSystem(new DirectionalVisibility_System(m_frameData, cameras));
		auxilliarySystems.addSystem(new DirectionalSync_System(m_frameData, cameras));

		// Asset Loading
		m_shader_Lighting = Shared_Shader(m_engine, "Core\\Directional\\Light");
		m_shader_Bounce = Shared_Shader(m_engine, "Core\\Directional\\Bounce");
		m_shapeQuad = Shared_Primitive(engine, "quad");

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

		// Clear state on world-unloaded
		m_engine->getModule_World().addLevelListener(m_aliveIndicator, [&](const World_Module::WorldState & state) {
			if (state == World_Module::unloaded)
				clear();
		});

		// Preferences
		auto & preferences = engine->getPreferenceState();
		preferences.getOrSetValue(PreferenceState::C_RH_BOUNCE_SIZE, m_bounceSize);
		preferences.addCallback(PreferenceState::C_RH_BOUNCE_SIZE, m_aliveIndicator, [&](const float &f) { m_bounceSize = (GLuint)f; });
	}


	// Public Interface Implementations
	inline virtual void prepareForNextFrame(const float & deltaTime) override {
		m_frameData->lightBuffer.endWriting();
		for (auto & drawBuffer : m_drawBuffers) {
			drawBuffer.visLights.endWriting();
			drawBuffer.indirectShape.endWriting();
			drawBuffer.indirectBounce.endWriting();
		}
		m_drawIndex = 0;
	}
	inline virtual void renderTechnique(const float & deltaTime, const std::shared_ptr<Viewport> & viewport, const std::vector<std::pair<int, int>> & perspectives) override {
		// Exit Early
		if (m_enabled && m_frameData->viewInfo.size() && m_shapeQuad->existsYet() && m_shader_Lighting->existsYet()) {
			if (m_drawIndex >= m_drawBuffers.size())
				m_drawBuffers.resize(m_drawIndex + 1);
			auto & drawBuffer = m_drawBuffers[m_drawIndex];
			auto &camBufferIndex = drawBuffer.bufferCamIndex;
			auto &lightBufferIndex = drawBuffer.visLights;
			camBufferIndex.beginWriting();
			lightBufferIndex.beginWriting();
			drawBuffer.indirectShape.beginWriting();
			drawBuffer.indirectBounce.beginWriting();

			// Accumulate all visibility info for the cameras passed in
			std::vector<glm::ivec2> camIndices; 
			std::vector<GLint> lightIndices;
			int shadowCount = 0;
			for (auto &[camIndex, layer] : perspectives) {
				const std::vector<glm::ivec2> tempIndices(m_frameData->viewInfo[camIndex].lightIndices.size(), { camIndex, layer });
				camIndices.insert(camIndices.end(), tempIndices.begin(), tempIndices.end());
				lightIndices.insert(lightIndices.end(), m_frameData->viewInfo[camIndex].lightIndices.begin(), m_frameData->viewInfo[camIndex].lightIndices.end());
				shadowCount += m_frameData->viewInfo[camIndex].visShadowCount;
			}

			if (lightIndices.size()) {
				// Write accumulated data
				camBufferIndex.write(0, sizeof(glm::ivec2) * camIndices.size(), camIndices.data());
				lightBufferIndex.write(0, sizeof(GLuint) * lightIndices.size(), lightIndices.data());
				const GLuint dataShape[] = { (GLuint)m_shapeQuad->getSize(), (GLuint)lightIndices.size(), 0,0 };
				const GLuint dataBounce[] = { (GLuint)m_shapeQuad->getSize(), (GLuint)(shadowCount * m_bounceSize),0,0 };
				drawBuffer.indirectShape.write(0, sizeof(GLuint) * 4, &dataShape);
				drawBuffer.indirectBounce.write(0, sizeof(GLuint) * 4, &dataBounce);
				drawBuffer.shadowCount = shadowCount;

				// Render lights
				renderLights(deltaTime, viewport);

				// Render indirect lights
				renderBounce(deltaTime, viewport);

				m_drawIndex++;
			}
		}
	}


private:
	// Private Methods
	/** Render all the lights.
	@param	deltaTime	the amount of time passed since last frame.
	@param	viewport	the viewport to render from. */
	inline void renderLights(const float & deltaTime, const std::shared_ptr<Viewport> & viewport) {
		glEnable(GL_BLEND);
		glBlendEquation(GL_FUNC_ADD);
		glBlendFunc(GL_ONE, GL_ONE);
		glDisable(GL_DEPTH_TEST);
		glDepthMask(GL_FALSE);

		m_shader_Lighting->bind();									// Shader (directional)
		viewport->m_gfxFBOS->bindForWriting("LIGHTING");			// Ensure writing to lighting FBO
		viewport->m_gfxFBOS->bindForReading("GEOMETRY", 0);			// Read from Geometry FBO
		glBindTextureUnit(4, m_frameData->shadowData->shadowFBO.m_texDepth);			// Shadow map (depth texture)
		m_drawBuffers[m_drawIndex].bufferCamIndex.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 3);	
		m_drawBuffers[m_drawIndex].visLights.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 4);	// SSBO visible light indices
		m_frameData->lightBuffer.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 8);
		m_drawBuffers[m_drawIndex].indirectShape.bindBuffer(GL_DRAW_INDIRECT_BUFFER);		// Draw call buffer
		glBindVertexArray(m_shapeQuad->m_vaoID);					// Quad VAO
		glDrawArraysIndirect(GL_TRIANGLES, 0);						// Now draw
	}
	/** Render light bounces.
	@param	deltaTime	the amount of time passed since last frame.
	@param	viewport	the viewport to render from. */
	inline void renderBounce(const float & deltaTime, const std::shared_ptr<Viewport> & viewport) {
		if (m_drawBuffers.size() > m_drawIndex && m_drawBuffers[m_drawIndex].shadowCount) {
			m_shader_Bounce->setUniform(0, (GLint)(m_drawBuffers[m_drawIndex].shadowCount));
			m_shader_Bounce->setUniform(1, m_rhVolume->m_max);
			m_shader_Bounce->setUniform(2, m_rhVolume->m_min);
			m_shader_Bounce->setUniform(4, m_rhVolume->m_resolution);
			m_shader_Bounce->setUniform(6, m_rhVolume->m_unitSize);

			// Prepare rendering state
			glBlendEquationSeparatei(0, GL_MIN, GL_MIN);
			glBindVertexArray(m_shapeQuad->m_vaoID);

			glViewport(0, 0, (GLsizei)m_rhVolume->m_resolution, (GLsizei)m_rhVolume->m_resolution);
			m_shader_Bounce->bind();
			m_rhVolume->writePrimary();
			glBindTextureUnit(0, m_frameData->shadowData->shadowFBO.m_texNormal);
			glBindTextureUnit(1, m_frameData->shadowData->shadowFBO.m_texColor);
			glBindTextureUnit(2, m_frameData->shadowData->shadowFBO.m_texDepth);
			glBindTextureUnit(3, viewport->m_gfxFBOS->getTexID("GEOMETRY", 3));
			glBindTextureUnit(4, m_textureNoise32);
			m_drawBuffers[m_drawIndex].bufferCamIndex.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 3);
			m_drawBuffers[m_drawIndex].visLights.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 4);
			m_frameData->lightBuffer.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 8);
			m_drawBuffers[m_drawIndex].indirectBounce.bindBuffer(GL_DRAW_INDIRECT_BUFFER);
			glDrawArraysIndirect(GL_TRIANGLES, 0);

			glDepthMask(GL_TRUE);
			glEnable(GL_DEPTH_TEST);
			glBlendEquation(GL_FUNC_ADD);
			glBlendFunc(GL_ONE, GL_ZERO);
			glDisable(GL_BLEND);
			glViewport(0, 0, GLsizei(viewport->m_dimensions.x), GLsizei(viewport->m_dimensions.y));
		}
	}
	/** Clear out the lights and shadows queued up for rendering. */
	inline void clear() {
		m_frameData->viewInfo.clear();
	}


	// Private Attributes
	Engine * m_engine = nullptr;
	std::shared_ptr<bool> m_aliveIndicator = std::make_shared<bool>(true);
	Shared_Shader m_shader_Lighting, m_shader_Bounce;
	Shared_Primitive m_shapeQuad;
	GLuint m_textureNoise32 = 0;
	GLuint m_bounceSize = 16u;
	struct DrawBuffers {
		DynamicBuffer bufferCamIndex;
		DynamicBuffer visLights;
		int shadowCount = 0;
		StaticTripleBuffer indirectShape = StaticTripleBuffer(sizeof(GLuint) * 4), indirectBounce = StaticTripleBuffer(sizeof(GLuint) * 4);
	};
	int m_drawIndex = 0;
	std::vector<DrawBuffers> m_drawBuffers;


	// Shared Attributes
	std::shared_ptr<DirectionalData> m_frameData;
	std::shared_ptr<RH_Volume> m_rhVolume;
	std::shared_ptr<std::vector<CameraBuffer::CamStruct*>> m_cameras;
};

#endif // DIRECTIONAL_TECHNIQUE_H