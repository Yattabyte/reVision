#pragma once
#ifndef LIGHTSPOT_FX_H
#define LIGHTSPOT_FX_H 

#include "Modules/Graphics/Effects/GFX_Core_Effect.h"
#include "Assets/Shader.h"
#include "Assets/Primitive.h"
#include "Modules/Graphics/ECS/LightSpot_S.h"
#include "Modules/Graphics/ECS/PropShadowing_S.h"
#include "Modules/Graphics/Common/FBO_Shadow_Spot.h"
#include "Modules/Graphics/Effects/PropShadowing_FX.h"
#include "Utilities/GL/FBO.h"
#include "Engine.h"
#include "GLFW/glfw3.h"


/** A core-rendering technique which applies spot lighting to the scene. */
class LightSpot_Effect : public GFX_Core_Effect {
public:
	// (de)Constructors
	/** Virtual Destructor. */
	~LightSpot_Effect() {
		// Update indicator
		m_aliveIndicator = false;
	}
	/** Constructor. */
	LightSpot_Effect(
		Engine * engine,
		FBO_Base * geometryFBO, FBO_Base * lightingFBO,
		GL_Vector * propBuffer, GL_Vector * skeletonBuffer,
		Spot_RenderState * renderState
	) : m_engine(engine), m_geometryFBO(geometryFBO), m_lightingFBO(lightingFBO), m_renderState(renderState) {
		// Asset Loading
		m_shader_Lighting = Shared_Shader(m_engine, "Core\\Spot\\Light");
		m_shader_Stencil = Shared_Shader(m_engine, "Core\\Spot\\Stencil");
		m_shader_Shadow = Shared_Shader(m_engine, "Core\\Spot\\Shadow");
		m_shader_Culling = Shared_Shader(m_engine, "Core\\Spot\\Culling");
		m_shapeCone = Shared_Primitive(m_engine, "cone");

		// Preferences
		auto & preferences = m_engine->getPreferenceState();
		preferences.getOrSetValue(PreferenceState::C_WINDOW_WIDTH, m_renderSize.x);
		preferences.addCallback(PreferenceState::C_WINDOW_WIDTH, m_aliveIndicator, [&](const float &f) {m_renderSize = glm::ivec2(f, m_renderSize.y); });
		preferences.getOrSetValue(PreferenceState::C_WINDOW_HEIGHT, m_renderSize.y);
		preferences.addCallback(PreferenceState::C_WINDOW_HEIGHT, m_aliveIndicator, [&](const float &f) {m_renderSize = glm::ivec2(m_renderSize.x, f); });
		preferences.getOrSetValue(PreferenceState::C_SHADOW_QUALITY, m_renderState->m_updateQuality);
		preferences.addCallback(PreferenceState::C_SHADOW_QUALITY, m_aliveIndicator, [&](const float &f) { m_renderState->m_updateQuality = (unsigned int)f; });
		preferences.getOrSetValue(PreferenceState::C_SHADOW_SIZE_SPOT, m_renderState->m_shadowSize.x);
		preferences.addCallback(PreferenceState::C_SHADOW_SIZE_SPOT, m_aliveIndicator, [&](const float &f) { m_renderState->m_shadowSize = glm::ivec2(std::max(1, (int)f)); });
		m_renderState->m_shadowSize = glm::ivec2(std::max(1, m_renderState->m_shadowSize.x));
		m_shadowFBO.resize(m_renderState->m_shadowSize, 1);

		// Asset-Finished Callbacks
		m_shapeCone->addCallback(m_aliveIndicator, [&]() mutable {
			const GLuint data = { (GLuint)m_shapeCone->getSize() };
			m_renderState->m_indirectShape.write(0, sizeof(GLuint), &data); // count, primCount, first, reserved
		});
		m_shader_Lighting->addCallback(m_aliveIndicator, [&](void) {m_shader_Lighting->setUniform(0, 1.0f / (float)m_renderState->m_shadowSize.x); });

		// Geometry rendering pipeline
		m_geometryStaticSystems.addSystem(new PropShadowing_System(engine, 1, PropShadowing_System::RenderStatic));
		m_geometryDynamicSystems.addSystem(new PropShadowing_System(engine, 1, PropShadowing_System::RenderDynamic));
		m_geometryEffectsStatic.push_back(new PropShadowing_Effect(engine, m_shader_Culling, m_shader_Shadow, propBuffer, skeletonBuffer, &((PropShadowing_System*)m_geometryStaticSystems[0])->m_renderState));
		m_geometryEffectsDynamic.push_back(new PropShadowing_Effect(engine, m_shader_Culling, m_shader_Shadow, propBuffer, skeletonBuffer, &((PropShadowing_System*)m_geometryDynamicSystems[0])->m_renderState));

		// Error Reporting
		if (glCheckNamedFramebufferStatus(m_shadowFBO.m_fboID, GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			m_engine->getManager_Messages().error("SpotLight Shadowmap Framebuffer has encountered an error.");
	}


	// Interface Implementation	
	inline virtual void applyEffect(const float & deltaTime) override {
		// Exit Early
		if (!m_shapeCone->existsYet() || !m_shader_Lighting->existsYet() || !m_shader_Stencil->existsYet() || !m_shader_Shadow->existsYet() || !m_shader_Culling->existsYet())
			return;

		// Bind buffers common for rendering and shadowing
		m_lightBuffer.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 8); // Light buffer
		m_shadowBuffer.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 9); // Shadow buffer

		// Render important shadows
		renderShadows(deltaTime);
		// Render lights
		renderLights(deltaTime);
	}
	

	// Public Attributes
	VectorBuffer<LightSpot_Buffer> m_lightBuffer;
	VectorBuffer<LightSpotShadow_Buffer> m_shadowBuffer;
	FBO_Shadow_Spot m_shadowFBO;


protected:
	// Protected Methods
	/** Render all the geometry from each light. */
	inline void renderShadows(const float & deltaTime) {
		auto & world = m_engine->getModule_World();
		glViewport(0, 0, m_renderState->m_shadowSize.x, m_renderState->m_shadowSize.y);
		m_shader_Shadow->bind();
		m_shadowFBO.bindForWriting();
		for each (const auto & pair in m_renderState->m_shadowsToUpdate) {
			glUniform1i(0, pair.first->m_data->index);
			glUniform1i(1, pair.second->m_data->index);
			// Update static shadows
			if (pair.second->m_outOfDate || m_renderState->m_outOfDate) {
				m_shadowFBO.clear(pair.second->m_shadowSpot + 1);
				world.updateSystems(m_geometryStaticSystems, deltaTime);
				for each (auto *tech in m_geometryEffectsStatic)
					if (tech->isEnabled())
						tech->applyEffect(deltaTime);
				pair.second->m_outOfDate = false;
			}
			// Update dynamic shadows
			m_shadowFBO.clear(pair.second->m_shadowSpot);
			world.updateSystems(m_geometryDynamicSystems, deltaTime);
			for each (auto *tech in m_geometryEffectsDynamic)
				if (tech->isEnabled())
					tech->applyEffect(deltaTime);
			pair.second->m_updateTime = (float)glfwGetTime();
		}

		if (m_renderState->m_outOfDate)
			m_renderState->m_outOfDate = false;
		glViewport(0, 0, m_renderSize.x, m_renderSize.y);
	}
	/** Render all the lights. */
	inline void renderLights(const float & deltaTime) {
		glEnable(GL_STENCIL_TEST);
		glEnable(GL_BLEND);
		glBlendEquation(GL_FUNC_ADD);
		glBlendFunc(GL_ONE, GL_ONE);

		// Draw only into depth-stencil buffer
		m_shader_Stencil->bind();													// Shader (spot)
		m_lightingFBO->bindForWriting();											// Ensure writing to lighting FBO
		m_geometryFBO->bindForReading();											// Read from Geometry FBO
		glBindTextureUnit(4, m_shadowFBO.m_textureIDS[2]);							// Shadow map (depth texture)
		m_renderState->m_visLights.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 3);		// SSBO visible light indices
		m_renderState->m_visShadows.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 4);	// SSBO visible shadow indices
		m_renderState->m_indirectShape.bindBuffer(GL_DRAW_INDIRECT_BUFFER);			// Draw call buffer
		glBindVertexArray(m_shapeCone->m_vaoID);									// Quad VAO
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

		glCullFace(GL_BACK);
		glDepthMask(GL_TRUE);
		glBlendFunc(GL_ONE, GL_ZERO);
		glDisable(GL_BLEND);
		glDisable(GL_STENCIL_TEST);
	}


private:
	// Private Attributes
	Engine * m_engine = nullptr;
	glm::ivec2 m_renderSize = glm::ivec2(1);
	Shared_Shader m_shader_Lighting, m_shader_Stencil, m_shader_Shadow, m_shader_Culling;
	Shared_Primitive m_shapeCone;
	ECSSystemList m_geometryStaticSystems, m_geometryDynamicSystems;
	std::vector<GFX_Core_Effect*> m_geometryEffectsStatic, m_geometryEffectsDynamic;
	FBO_Base * m_geometryFBO = nullptr, * m_lightingFBO = nullptr;
	Spot_RenderState * m_renderState = nullptr;
	std::shared_ptr<bool> m_aliveIndicator = std::make_shared<bool>(true);
};

#endif // LIGHTSPOT_FX_H