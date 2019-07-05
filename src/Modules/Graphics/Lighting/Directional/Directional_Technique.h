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
	inline Directional_Technique(Engine * engine, const std::shared_ptr<ShadowData> & shadowData, const std::shared_ptr<CameraBuffer> & clientCamera, const std::shared_ptr<std::vector<CameraBuffer::CamStruct*>> & cameras, ECSSystemList & auxilliarySystems)
		: m_engine(engine), m_cameras(cameras), Graphics_Technique(PRIMARY_LIGHTING) {
		// Auxilliary Systems
		m_frameData = std::make_shared<DirectionalData>();
		m_frameData->clientCamera = clientCamera;
		m_frameData->shadowData = shadowData;
		auxilliarySystems.addSystem(new DirectionalVisibility_System(m_engine, m_frameData, cameras));
		auxilliarySystems.addSystem(new DirectionalSync_System(m_frameData, cameras));

		// Asset Loading
		m_shader_Lighting = Shared_Shader(m_engine, "Core\\Directional\\Light");
		m_shader_Bounce = Shared_Shader(m_engine, "Core\\Directional\\Bounce");
		m_shapeQuad = Shared_Primitive(engine, "quad");

		// Asset-Finished Callbacks
		m_shapeQuad->addCallback(m_aliveIndicator, [&]() mutable {
			m_frameData->shapeVertexCount = m_shapeQuad->getSize();
		});

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
	}


	// Public Interface Implementations
	inline virtual void prepareForNextFrame(const float & deltaTime) override {
		m_frameData->lightBuffer.endWriting();
		for (auto & viewInfo : m_frameData->viewInfo)
			viewInfo.visLights.endWriting();
	}
	inline virtual void renderTechnique(const float & deltaTime, const std::shared_ptr<Viewport> & viewport, const CameraBuffer::CamStruct * camera) override {
		// Exit Early
		if (m_enabled && m_shapeQuad->existsYet() && m_shader_Lighting->existsYet() && m_shader_Bounce->existsYet()) {
			size_t visibilityIndex = 0;
			bool found = false;
			for (size_t x = 0; x < m_cameras->size(); ++x)
				if (m_cameras->at(x) == camera) {
					visibilityIndex = x;
					found = true;
					break;
				}
			if (found) {
				// Render lights
				renderLights(deltaTime, viewport, visibilityIndex);

				// Render indirect lights
				renderBounce(deltaTime, viewport, visibilityIndex);
			}
		}
	}


private:
	// Private Methods
	/** Render all the lights.
	@param	deltaTime	the amount of time passed since last frame.
	@param	viewport	the viewport to render from. */
	inline void renderLights(const float & deltaTime, const std::shared_ptr<Viewport> & viewport, const size_t & visibilityIndex) {
		if (m_frameData->viewInfo.size() && m_frameData->viewInfo[visibilityIndex].visLightCount) {
			glEnable(GL_BLEND);
			glBlendEquation(GL_FUNC_ADD);
			glBlendFunc(GL_ONE, GL_ONE);
			glDisable(GL_DEPTH_TEST);
			glDepthMask(GL_FALSE);

			m_shader_Lighting->bind();									// Shader (directional)
			viewport->m_gfxFBOS->bindForWriting("LIGHTING");			// Ensure writing to lighting FBO
			viewport->m_gfxFBOS->bindForReading("GEOMETRY", 0);			// Read from Geometry FBO
			glBindTextureUnit(4, m_frameData->shadowData->shadowFBO.m_texDepth);			// Shadow map (depth texture)
			m_frameData->viewInfo[visibilityIndex].visLights.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 3);	// SSBO visible light indices
			m_frameData->lightBuffer.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 8);
			m_frameData->viewInfo[visibilityIndex].indirectShape.bindBuffer(GL_DRAW_INDIRECT_BUFFER);		// Draw call buffer
			glBindVertexArray(m_shapeQuad->m_vaoID);					// Quad VAO
			glDrawArraysIndirect(GL_TRIANGLES, 0);						// Now draw
		}

	}
	/** Render light bounces.
	@param	deltaTime	the amount of time passed since last frame.
	@param	viewport	the viewport to render from. */
	inline void renderBounce(const float & deltaTime, const std::shared_ptr<Viewport> & viewport, const size_t & visibilityIndex) {
		if (m_frameData->viewInfo.size() && m_frameData->viewInfo[visibilityIndex].visShadowCount) {
			m_shader_Bounce->setUniform(0, (GLint)(m_frameData->viewInfo[visibilityIndex].visShadowCount));
			m_shader_Bounce->setUniform(1, viewport->m_rhVolume->m_max);
			m_shader_Bounce->setUniform(2, viewport->m_rhVolume->m_min);
			m_shader_Bounce->setUniform(4, viewport->m_rhVolume->m_resolution);
			m_shader_Bounce->setUniform(6, viewport->m_rhVolume->m_unitSize);

			// Prepare rendering state
			glBlendEquationSeparatei(0, GL_MIN, GL_MIN);
			glBindVertexArray(m_shapeQuad->m_vaoID);

			glViewport(0, 0, (GLsizei)viewport->m_rhVolume->m_resolution, (GLsizei)viewport->m_rhVolume->m_resolution);
			m_shader_Bounce->bind();
			viewport->m_rhVolume->writePrimary();
			viewport->m_gfxFBOS->bindForReading("GEOMETRY", 0);
			glBindTextureUnit(0, m_frameData->shadowData->shadowFBO.m_texNormal);
			glBindTextureUnit(1, m_frameData->shadowData->shadowFBO.m_texColor);
			glBindTextureUnit(2, m_frameData->shadowData->shadowFBO.m_texDepth);
			glBindTextureUnit(4, m_textureNoise32);
			m_frameData->viewInfo[visibilityIndex].visLights.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 3);		// SSBO visible light indices
			m_frameData->lightBuffer.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 8);
			m_frameData->viewInfo[visibilityIndex].indirectBounce.bindBuffer(GL_DRAW_INDIRECT_BUFFER);
			glDrawArraysIndirect(GL_TRIANGLES, 0);
			glViewport(0, 0, GLsizei(viewport->m_dimensions.x), GLsizei(viewport->m_dimensions.y));

			glDepthMask(GL_TRUE);
			glEnable(GL_DEPTH_TEST);
			glBlendEquation(GL_FUNC_ADD);
			glBlendFunc(GL_ONE, GL_ZERO);
			glDisable(GL_BLEND);
		}
	}
	/** Clear out the lights and shadows queued up for rendering. */
	inline void clear() {
		m_frameData->viewInfo.clear();
		m_frameData->lightBuffer.clear();
	}


	// Private Attributes
	Engine * m_engine = nullptr;
	std::shared_ptr<bool> m_aliveIndicator = std::make_shared<bool>(true);
	Shared_Shader m_shader_Lighting, m_shader_Bounce;
	Shared_Primitive m_shapeQuad;
	GLuint m_textureNoise32 = 0;


	// Shared Attributes
	std::shared_ptr<DirectionalData> m_frameData;
	std::shared_ptr<std::vector<CameraBuffer::CamStruct*>> m_cameras;
};

#endif // DIRECTIONAL_TECHNIQUE_H