#pragma once
#ifndef POINT_TECHNIQUE_H
#define POINT_TECHNIQUE_H

#include "Modules/Graphics/Common/Graphics_Technique.h"
#include "Modules/Graphics/Lighting/components.h"
#include "Modules/Graphics/Lighting/Point/PointData.h"
#include "Modules/Graphics/Lighting/Point/PointVisibility_System.h"
#include "Modules/Graphics/Lighting/Point/PointScheduler_System.h"
#include "Modules/Graphics/Lighting/Point/PointSync_System.h"
#include "Modules/World/ECS/components.h"
#include "Modules/World/ECS/ecsSystem.h"
#include "Assets/Shader.h"
#include "Assets/Primitive.h"
#include "Engine.h"


/** A core lighting technique responsible for all point lights. */
class Point_Technique : public Graphics_Technique {
public:
	// Public (de)Constructors
	/** Destructor. */
	inline ~Point_Technique() {
		// Update indicator
		m_aliveIndicator = false;
	}
	/** Constructor. */
	inline Point_Technique(Engine * engine, const std::shared_ptr<std::vector<std::shared_ptr<CameraBuffer>>> & cameras, ECSSystemList & auxilliarySystems)
		: m_engine(engine), m_cameras(cameras), Graphics_Technique(PRIMARY_LIGHTING) {
		// Auxilliary Systems
		m_frameData = std::make_shared<PointData>();
		auxilliarySystems.addSystem(new PointScheduler_System(m_engine, m_frameData));
		auxilliarySystems.addSystem(new PointVisibility_System(m_frameData, cameras));
		auxilliarySystems.addSystem(new PointSync_System(m_frameData));

		// Asset Loading
		m_shader_Lighting = Shared_Shader(m_engine, "Core\\Point\\Light");
		m_shader_Stencil = Shared_Shader(m_engine, "Core\\Point\\Stencil");
		m_shapeSphere = Shared_Primitive(m_engine, "sphere");

		// Preferences
		auto & preferences = m_engine->getPreferenceState();
		preferences.getOrSetValue(PreferenceState::C_SHADOW_SIZE_POINT, m_frameData->shadowSize.x);
		preferences.addCallback(PreferenceState::C_SHADOW_SIZE_POINT, m_aliveIndicator, [&](const float &f) {
			m_frameData->shadowSize = glm::ivec2(std::max(1, (int)f));
			if (m_shader_Lighting && m_shader_Lighting->existsYet())
				m_shader_Lighting->addCallback(m_aliveIndicator, [&](void) { m_shader_Lighting->setUniform(0, 1.0f / (float)m_frameData->shadowSize.x); });
		});
		m_frameData->shadowSize = glm::ivec2(std::max(1, m_frameData->shadowSize.x));

		// Asset-Finished Callbacks
		m_shapeSphere->addCallback(m_aliveIndicator, [&]() mutable {
			m_frameData->shapeVertexCount = m_shapeSphere->getSize();
		});
		m_shader_Lighting->addCallback(m_aliveIndicator, [&](void) { m_shader_Lighting->setUniform(0, 1.0f / (float)m_frameData->shadowSize.x); });

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
	inline virtual void updateTechnique(const float & deltaTime) override {
		// Render important shadows
		if (m_enabled)
			updateShadows(deltaTime);
	}
	inline virtual void renderTechnique(const float & deltaTime, const std::shared_ptr<Viewport> & viewport, const std::shared_ptr<CameraBuffer> & camera) override {
		// Render direct lights	
		if (m_enabled && m_shapeSphere->existsYet() && m_shader_Lighting->existsYet() && m_shader_Stencil->existsYet()) {
			size_t visibilityIndex = 0;
			bool found = false;
			for (size_t x = 0; x < m_cameras->size(); ++x)
				if (m_cameras->at(x).get() == camera.get()) {
					visibilityIndex = x;
					found = true;
					break;
				}
			if (found)
				renderLights(deltaTime, viewport, visibilityIndex);
		}
	}


private:
	// Private Methods	
	/** Render all the geometry from each light.
	@param	deltaTime	the amount of time passed since last frame.*/
	inline void updateShadows(const float & deltaTime) {
		if (m_frameData->shadowsToUpdate.size()) {
			glViewport(0, 0, m_frameData->shadowSize.x, m_frameData->shadowSize.y);
			m_frameData->shadowFBO.bindForWriting();
			for (auto &[time, light, cameras] : (m_frameData->shadowsToUpdate)) {
				if (cameras.size() == 6) {
					m_frameData->shadowFBO.clear(light->m_shadowSpot);
					// Render all six sides
					for (int x = 0; x < 6; ++x) {
						// Prepare Camera
						cameras[x]->beginWriting();
						cameras[x]->pushChanges();

						// Update shadows
						m_engine->getModule_Graphics().renderShadows(deltaTime, cameras[x], light->m_shadowSpot + x , glm::vec3(1.0f));

						// End Camera
						cameras[x]->endWriting();
					}
				}
				light->m_updateTime += deltaTime;
			}
			m_frameData->shadowsToUpdate.clear();
		}
	}
	/** Render all the lights.
	@param	deltaTime	the amount of time passed since last frame.
	@param	viewport	the viewport to render from. */
	inline void renderLights(const float & deltaTime, const std::shared_ptr<Viewport> & viewport, const size_t & visibilityIndex) {
		if (m_frameData->viewInfo.size() && m_frameData->viewInfo[visibilityIndex].visLightCount) {
			glEnable(GL_STENCIL_TEST);
			glEnable(GL_BLEND);
			glBlendEquation(GL_FUNC_ADD);
			glBlendFunc(GL_ONE, GL_ONE);

			// Draw only into depth-stencil buffer
			m_shader_Stencil->bind();																		// Shader (point)
			viewport->m_gfxFBOS->bindForWriting("LIGHTING");												// Ensure writing to lighting FBO
			viewport->m_gfxFBOS->bindForReading("GEOMETRY", 0);												// Read from Geometry FBO
			glBindTextureUnit(4, m_frameData->shadowFBO.m_textureIDS[3]);									// Shadow map(linear depth texture)
			m_frameData->viewInfo[visibilityIndex].visLights.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 3);	// SSBO visible light indices
			m_frameData->lightBuffer.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 8);
			m_frameData->viewInfo[visibilityIndex].indirectShape.bindBuffer(GL_DRAW_INDIRECT_BUFFER);		// Draw call buffer
			glBindVertexArray(m_shapeSphere->m_vaoID);	
			glDepthMask(GL_FALSE);
			glEnable(GL_DEPTH_TEST);
			glDisable(GL_CULL_FACE);
			glClear(GL_STENCIL_BUFFER_BIT);
			glStencilFunc(GL_ALWAYS, 0, 0);
			glDrawArraysIndirect(GL_TRIANGLES, 0);

			// Now draw into color buffers
			m_shader_Lighting->bind();
			glDisable(GL_DEPTH_TEST);
			glEnable(GL_CULL_FACE);
			glCullFace(GL_FRONT);
			glStencilFunc(GL_NOTEQUAL, 0, 0xFF);
			glDrawArraysIndirect(GL_TRIANGLES, 0);

			glClear(GL_STENCIL_BUFFER_BIT);
			glCullFace(GL_BACK);
			glDepthMask(GL_TRUE);
			glBlendFunc(GL_ONE, GL_ZERO);
			glDisable(GL_BLEND);
			glDisable(GL_STENCIL_TEST);
		}
	}
	/** Clear out the lights and shadows queued up for rendering. */
	inline void clear() {
		m_frameData->viewInfo.clear();
		m_frameData->shadowsToUpdate.clear();
		m_frameData->lightBuffer.clear();
	}


	// Private Attributes
	Engine * m_engine = nullptr;
	std::shared_ptr<bool> m_aliveIndicator = std::make_shared<bool>(true);
	Shared_Shader m_shader_Lighting, m_shader_Stencil;
	Shared_Primitive m_shapeSphere;


	// Shared Attributes
	std::shared_ptr<PointData> m_frameData;
	std::shared_ptr<std::vector<std::shared_ptr<CameraBuffer>>> m_cameras;
};

#endif // POINT_TECHNIQUE_H