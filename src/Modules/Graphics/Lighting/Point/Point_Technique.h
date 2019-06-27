#pragma once
#ifndef POINT_TECHNIQUE_H
#define POINT_TECHNIQUE_H

#include "Modules/Graphics/Common/Graphics_Technique.h"
#include "Modules/Graphics/Lighting/components.h"
#include "Modules/Graphics/Lighting/Point/FBO_Shadow_Point.h"
#include "Modules/Graphics/Lighting/Point/PointVisibility_System.h"
#include "Modules/Graphics/Lighting/Point/PointScheduler_System.h"
#include "Modules/Graphics/Lighting/Point/PointSync_System.h"
#include "Modules/Graphics/Geometry/Prop/Prop_Shadow.h"
#include "Modules/World/ECS/components.h"
#include "Modules/World/ECS/ecsSystem.h"
#include "Assets/Shader.h"
#include "Assets/Primitive.h"
#include "Engine.h"
#include <vector>


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
	inline Point_Technique(Engine * engine, const std::shared_ptr<std::vector<Viewport*>> & viewports, Prop_Technique * propView, ECSSystemList & auxilliarySystems)
		: m_engine(engine), m_viewports(viewports), Graphics_Technique(PRIMARY_LIGHTING) {
		// Auxilliary Systems
		m_visibility = std::make_shared<Point_Visibility>();
		m_shadowsToUpdate = std::make_shared<std::vector<std::pair<float, LightPoint_Component*>>>();
		m_buffers = std::make_shared<Point_Buffers>();
		auxilliarySystems.addSystem(new PointVisibility_System(m_visibility, viewports));
		auxilliarySystems.addSystem(new PointScheduler_System(m_engine, m_shadowsToUpdate));
		auxilliarySystems.addSystem(new PointSync_System(m_buffers));

		// Asset Loading
		m_shader_Lighting = Shared_Shader(m_engine, "Core\\Point\\Light");
		m_shader_Stencil = Shared_Shader(m_engine, "Core\\Point\\Stencil");
		m_shader_Shadow = Shared_Shader(m_engine, "Core\\Point\\Shadow");
		m_shader_Culling = Shared_Shader(m_engine, "Core\\Point\\Culling");
		m_shapeSphere = Shared_Primitive(m_engine, "sphere");

		// Preferences
		auto & preferences = m_engine->getPreferenceState();
		preferences.getOrSetValue(PreferenceState::C_SHADOW_SIZE_POINT, m_shadowSize.x);
		preferences.addCallback(PreferenceState::C_SHADOW_SIZE_POINT, m_aliveIndicator, [&](const float &f) {
			m_buffers->shadowOutOfDate = true;
			m_shadowSize = glm::ivec2(std::max(1, (int)f));
			m_shadowFBO.resize(m_shadowSize, m_shadowCount * 12);
			if (m_shader_Lighting && m_shader_Lighting->existsYet())
				m_shader_Lighting->addCallback(m_aliveIndicator, [&](void) { m_shader_Lighting->setUniform(0, 1.0f / (float)m_shadowSize.x); });
		});
		m_shadowSize = glm::ivec2(std::max(1, m_shadowSize.x));
		m_shadowFBO.resize(m_shadowSize, 6);

		// Asset-Finished Callbacks
		m_shapeSphere->addCallback(m_aliveIndicator, [&]() mutable {
			m_visibility->shapeVertexCount = m_shapeSphere->getSize();
		});
		m_shader_Lighting->addCallback(m_aliveIndicator, [&](void) { m_shader_Lighting->setUniform(0, 1.0f / (float)m_shadowSize.x); });

		// Geometry rendering pipeline
		m_propShadow_Static = new Prop_Shadow(m_engine, 6, Prop_Shadow::RenderStatic, m_shader_Culling, m_shader_Shadow, propView);
		m_propShadow_Dynamic = new Prop_Shadow(m_engine, 6, Prop_Shadow::RenderDynamic, m_shader_Culling, m_shader_Shadow, propView);

		// Error Reporting
		if (glCheckNamedFramebufferStatus(m_shadowFBO.m_fboID, GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			m_engine->getManager_Messages().error("Point_Technique Shadowmap Framebuffer has encountered an error.");

		// Add New Component Types
		auto & world = m_engine->getModule_World();
		world.addNotifyOnComponentType(LightPoint_Component::ID, m_aliveIndicator, [&](BaseECSComponent * c) {
			auto * lightComponent = (LightPoint_Component*)c;
			lightComponent->m_lightIndex = m_buffers->lightBuffer.newElement();

			// Assign shadowmap spot
			int shadowSpot = (int)(m_shadowCount) * 12;
			lightComponent->m_shadowSpot = shadowSpot;
			m_shadowCount++;
			m_shadowFBO.resize(m_shadowSize, m_shadowCount * 12);
		});

		// World-Changed Callback
		world.addLevelListener(m_aliveIndicator, [&](const World_Module::WorldState & state) {
			if (state == World_Module::unloaded) {
				clear();
				m_buffers->shadowOutOfDate = false;
			}
			else if (state == World_Module::finishLoading || state == World_Module::updated)
				m_buffers->shadowOutOfDate = true;
		});
	}


	// Public Interface Implementations
	inline virtual void beginFrame(const float & deltaTime) override {
		m_buffers->lightBuffer.beginWriting();
	}
	inline virtual void endFrame(const float & deltaTime) override {
		m_buffers->lightBuffer.endWriting();
		for (auto & viewInfo : m_visibility->viewInfo)
			viewInfo.visLights.endWriting();
	}
	inline virtual void updateTechnique(const float & deltaTime) override {
		// Exit Early
		if (!m_enabled || !m_shader_Shadow->existsYet() || !m_shader_Culling->existsYet())
			return;

		// Render important shadows
		updateShadows(deltaTime);
	}
	inline virtual void renderTechnique(const float & deltaTime, const std::shared_ptr<Viewport> & viewport) override {
		// Exit Early
		if (!m_enabled || !m_shapeSphere->existsYet() || !m_shader_Lighting->existsYet() || !m_shader_Stencil->existsYet())
			return;

		size_t visibilityIndex = 0;
		bool found = false;
		for (size_t x = 0; x < m_viewports->size(); ++x)
			if (m_viewports->at(x) == viewport.get()) {
				visibilityIndex = x;
				found = true;
				break;
			}

		if (found) {
			// Render direct lights
			renderLights(deltaTime, viewport, visibilityIndex);
		}
	}


private:
	// Private Methods	
	/** Render all the geometry from each light.
	@param	deltaTime	the amount of time passed since last frame.*/
	inline void updateShadows(const float & deltaTime) {
		if (m_shadowsToUpdate->size()) {
			m_buffers->lightBuffer.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 8);
			glViewport(0, 0, m_shadowSize.x, m_shadowSize.y);
			m_shadowFBO.bindForWriting();
			for (auto &[time, light] : (*m_shadowsToUpdate)) {
				// Update static shadows
				if (light->m_staticOutOfDate) {
					m_shadowFBO.clear(light->m_shadowSpot + 6);
					// Render components
					m_propShadow_Static->setData(light->m_position, (int)*light->m_lightIndex);
					m_propShadow_Static->renderShadows(deltaTime);
					light->m_staticOutOfDate = false;
				}
				// Update dynamic shadows
				m_shadowFBO.clear(light->m_shadowSpot);
				// Render components
				m_propShadow_Dynamic->setData(light->m_position, (int)*light->m_lightIndex);
				m_propShadow_Dynamic->renderShadows(deltaTime);
				light->m_updateTime = m_engine->getTime();
			}
			m_shadowsToUpdate->clear();
			m_buffers->shadowOutOfDate = false;
		}
	}
	/** Render all the lights.
	@param	deltaTime	the amount of time passed since last frame.
	@param	viewport	the viewport to render from. */
	inline void renderLights(const float & deltaTime, const std::shared_ptr<Viewport> & viewport, const size_t & visibilityIndex) {
		if (m_visibility->viewInfo[visibilityIndex].visLightCount) {
			glEnable(GL_STENCIL_TEST);
			glEnable(GL_BLEND);
			glBlendEquation(GL_FUNC_ADD);
			glBlendFunc(GL_ONE, GL_ONE);

			// Draw only into depth-stencil buffer
			m_shader_Stencil->bind();									// Shader (point)
			viewport->m_gfxFBOS->bindForWriting("LIGHTING");			// Ensure writing to lighting FBO
			viewport->m_gfxFBOS->bindForReading("GEOMETRY", 0);			// Read from Geometry FBO
			glBindTextureUnit(4, m_shadowFBO.m_textureIDS[0]);			// Shadow map(linear depth texture)
			m_visibility->viewInfo[visibilityIndex].visLights.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 3);	// SSBO visible light indices
			m_buffers->lightBuffer.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 8);
			m_visibility->viewInfo[visibilityIndex].indirectShape.bindBuffer(GL_DRAW_INDIRECT_BUFFER);		// Draw call buffer
			glBindVertexArray(m_shapeSphere->m_vaoID);					// Quad VAO
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
		m_visibility->viewInfo.clear();
		m_shadowsToUpdate->clear();
		m_buffers->lightBuffer.clear();
	}


	// Private Attributes
	Engine * m_engine = nullptr;
	std::shared_ptr<bool> m_aliveIndicator = std::make_shared<bool>(true);
	Shared_Shader m_shader_Lighting, m_shader_Stencil, m_shader_Shadow, m_shader_Culling;
	Shared_Primitive m_shapeSphere;
	Prop_Shadow * m_propShadow_Static = nullptr, *m_propShadow_Dynamic = nullptr;
	glm::ivec2 m_shadowSize = glm::ivec2(512);
	size_t m_shadowCount = 0ull;
	FBO_Shadow_Point m_shadowFBO;


	// Shared Attributes
	std::shared_ptr<Point_Buffers> m_buffers;
	std::shared_ptr<Point_Visibility> m_visibility;
	std::shared_ptr<std::vector<Viewport*>> m_viewports;
	std::shared_ptr<std::vector<std::pair<float, LightPoint_Component*>>> m_shadowsToUpdate;
};

#endif // POINT_TECHNIQUE_H