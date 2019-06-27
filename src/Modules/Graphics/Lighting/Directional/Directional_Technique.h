#pragma once
#ifndef DIRECTIONAL_TECHNIQUE_H
#define DIRECTIONAL_TECHNIQUE_H

#include "Modules/Graphics/Common/Graphics_Technique.h"
#include "Modules/Graphics/Logical/components.h"
#include "Modules/Graphics/Lighting/components.h"
#include "Modules/Graphics/Lighting/Directional/FBO_Shadow_Directional.h"
#include "Modules/Graphics/Lighting/Directional/DirectionalVisibility_System.h"
#include "Modules/Graphics/Lighting/Directional/DirectionalSync_System.h"
#include "Modules/Graphics/Geometry/Prop/Prop_Shadow.h"
#include "Modules/World/ECS/components.h"
#include "Modules/World/ECS/ecsSystem.h"
#include "Assets/Shader.h"
#include "Assets/Primitive.h"
#include "Engine.h"
#include <random>
#include <vector>

#define NUM_CASCADES 4


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
	inline Directional_Technique(Engine * engine, const std::shared_ptr<std::vector<Viewport*>> & viewports, Prop_Technique * propView, ECSSystemList & auxilliarySystems)
		: m_engine(engine), m_viewports(viewports), Graphics_Technique(PRIMARY_LIGHTING) {
		// Auxilliary Systems
		m_visibility = std::make_shared<Directional_Visibility>();
		m_buffers = std::make_shared<Directional_Buffers>();
		auxilliarySystems.addSystem(new DirectionalVisibility_System(m_engine, m_visibility, viewports));
		auxilliarySystems.addSystem(new DirectionalSync_System(m_buffers));

		// Asset Loading
		m_shader_Lighting = Shared_Shader(m_engine, "Core\\Directional\\Light");
		m_shader_Shadow = Shared_Shader(m_engine, "Core\\Directional\\Shadow");
		m_shader_Culling = Shared_Shader(m_engine, "Core\\Directional\\Culling");
		m_shader_Bounce = Shared_Shader(m_engine, "Core\\Directional\\Bounce");
		m_shapeQuad = Shared_Primitive(engine, "quad");

		// Preferences
		auto & preferences = m_engine->getPreferenceState();
		preferences.getOrSetValue(PreferenceState::C_SHADOW_SIZE_DIRECTIONAL, m_shadowSize.x);
		preferences.addCallback(PreferenceState::C_SHADOW_SIZE_DIRECTIONAL, m_aliveIndicator, [&](const float &f) {
			m_shadowSize = glm::ivec2(std::max(1, (int)f));
			m_shadowFBO.resize(m_shadowSize, m_shadowCount * 4);
			m_buffers->shadowSize = f;
			if (m_shader_Lighting && m_shader_Lighting->existsYet())
				m_shader_Lighting->addCallback(m_aliveIndicator, [&](void) { m_shader_Lighting->setUniform(0, 1.0f / m_shadowSize.x); });
		});
		m_shadowSize = glm::ivec2(std::max(1, m_shadowSize.x));
		m_shadowFBO.resize(m_shadowSize, 4);
		m_buffers->shadowSize = m_shadowSize.x;

		// Asset-Finished Callbacks
		m_shapeQuad->addCallback(m_aliveIndicator, [&]() mutable {
			m_visibility->shapeVertexCount = m_shapeQuad->getSize();
		});
		m_shader_Lighting->addCallback(m_aliveIndicator, [&](void) { m_shader_Lighting->setUniform(0, 1.0f / m_shadowSize.x); });

		// Geometry rendering pipeline
		m_propShadowSystem = new Prop_Shadow(engine, 4, Prop_Shadow::RenderAll, m_shader_Culling, m_shader_Shadow, propView);

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

		// Error Reporting
		if (glCheckNamedFramebufferStatus(m_shadowFBO.m_fboID, GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			m_engine->getManager_Messages().error("Directional_Technique Shadowmap Framebuffer has encountered an error.");

		// Add New Component Types
		auto & world = m_engine->getModule_World();
		world.addNotifyOnComponentType(LightDirectional_Component::ID, m_aliveIndicator, [&](BaseECSComponent * c) {
			auto * lightComponent = (LightDirectional_Component*)c;
			lightComponent->m_lightIndex = m_buffers->lightBuffer.newElement();

			// Assign shadowmap spot
			int shadowSpot = (int)(m_shadowCount) * 4;
			lightComponent->m_shadowSpot = shadowSpot;
			m_shadowCount++;
			m_shadowFBO.resize(m_shadowSize, m_shadowCount * 4);
		});

		// World-Changed Callback
		world.addLevelListener(m_aliveIndicator, [&](const World_Module::WorldState & state) {
			if (state == World_Module::unloaded)
				clear();
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
	inline virtual void renderTechnique(const float & deltaTime, const std::shared_ptr<Viewport> & viewport) override {
		// Exit Early
		if (!m_enabled || !m_shapeQuad->existsYet() || !m_shader_Lighting->existsYet() || !m_shader_Shadow->existsYet() || !m_shader_Culling->existsYet() || !m_shader_Bounce->existsYet())
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
			// Render important shadows
			renderShadows(deltaTime, viewport, visibilityIndex);

			// Render lights
			renderLights(deltaTime, viewport, visibilityIndex);

			// Render indirect lights
			renderBounce(deltaTime, viewport, visibilityIndex);
		}
	}


private:
	// Private Methods
	/** Render all the geometry from each light.
	@param	deltaTime	the amount of time passed since last frame.
	@param	viewport	the viewport to render from. */
	inline void renderShadows(const float & deltaTime, const std::shared_ptr<Viewport> & viewport, const size_t & visibilityIndex) {
		if (m_visibility->viewInfo[visibilityIndex].shadowsToUpdate.size()) {
			glViewport(0, 0, m_shadowSize.x, m_shadowSize.y);
			m_buffers->lightBuffer.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 8);
			m_shader_Shadow->bind();
			m_shadowFBO.bindForWriting();
			for (auto * light : m_visibility->viewInfo[visibilityIndex].shadowsToUpdate) {
				const float clearDepth(1.0f);
				const glm::vec3 clear(0.0f);
				m_shadowFBO.clear(light->m_shadowSpot);
				// Render geometry components
				m_propShadowSystem->setData(viewport->get3DPosition(), (int)*light->m_lightIndex);
				m_propShadowSystem->renderShadows(deltaTime);
				light->m_updateTime = m_engine->getTime();
			}
			m_visibility->viewInfo[visibilityIndex].shadowsToUpdate.clear();
			glViewport(0, 0, GLsizei(viewport->m_dimensions.x), GLsizei(viewport->m_dimensions.y));
		}
	}
	/** Render all the lights.
	@param	deltaTime	the amount of time passed since last frame.
	@param	viewport	the viewport to render from. */
	inline void renderLights(const float & deltaTime, const std::shared_ptr<Viewport> & viewport, const size_t & visibilityIndex) {
		if (m_visibility->viewInfo[visibilityIndex].visLightCount) {
			glEnable(GL_BLEND);
			glBlendEquation(GL_FUNC_ADD);
			glBlendFunc(GL_ONE, GL_ONE);
			glDisable(GL_DEPTH_TEST);
			glDepthMask(GL_FALSE);

			m_shader_Lighting->bind();									// Shader (directional)
			viewport->m_gfxFBOS->bindForWriting("LIGHTING");			// Ensure writing to lighting FBO
			viewport->m_gfxFBOS->bindForReading("GEOMETRY", 0);			// Read from Geometry FBO
			glBindTextureUnit(4, m_shadowFBO.m_textureIDS[2]);			// Shadow map (depth texture)
			m_visibility->viewInfo[visibilityIndex].visLights.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 3);	// SSBO visible light indices
			m_buffers->lightBuffer.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 8);
			m_visibility->viewInfo[visibilityIndex].indirectShape.bindBuffer(GL_DRAW_INDIRECT_BUFFER);		// Draw call buffer
			glBindVertexArray(m_shapeQuad->m_vaoID);					// Quad VAO
			glDrawArraysIndirect(GL_TRIANGLES, 0);						// Now draw
		}

	}
	/** Render light bounces.
	@param	deltaTime	the amount of time passed since last frame.
	@param	viewport	the viewport to render from.*/
	inline void renderBounce(const float & deltaTime, const std::shared_ptr<Viewport> & viewport, const size_t & visibilityIndex) {
		if (m_visibility->viewInfo[visibilityIndex].visShadowCount) {
			m_shader_Bounce->setUniform(0, (GLint)(m_visibility->viewInfo[visibilityIndex].visShadowCount));
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
			viewport->m_gfxFBOS->bindForReading("GEOMETRY", 0); // depth -> 3		
			glBindTextureUnit(0, m_shadowFBO.m_textureIDS[0]);
			glBindTextureUnit(1, m_shadowFBO.m_textureIDS[1]);
			glBindTextureUnit(2, m_shadowFBO.m_textureIDS[2]);
			glBindTextureUnit(4, m_textureNoise32);
			m_visibility->viewInfo[visibilityIndex].visLights.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 3);		// SSBO visible light indices
			m_buffers->lightBuffer.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 8);
			m_visibility->viewInfo[visibilityIndex].indirectBounce.bindBuffer(GL_DRAW_INDIRECT_BUFFER);
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
		m_visibility->viewInfo.clear();
		m_buffers->lightBuffer.clear();
	}


	// Private Attributes
	Engine * m_engine = nullptr;
	std::shared_ptr<bool> m_aliveIndicator = std::make_shared<bool>(true);
	Shared_Shader m_shader_Lighting, m_shader_Shadow, m_shader_Culling, m_shader_Bounce;
	Shared_Primitive m_shapeQuad;
	Prop_Shadow * m_propShadowSystem = nullptr;
	GLuint m_textureNoise32 = 0;
	glm::ivec2 m_shadowSize = glm::ivec2(1024);
	size_t m_shadowCount = 0ull;
	FBO_Shadow_Directional m_shadowFBO;;

	// Shared Attributes
	std::shared_ptr<Directional_Buffers> m_buffers;
	std::shared_ptr<Directional_Visibility> m_visibility;
	std::shared_ptr<std::vector<Viewport*>> m_viewports;
};

#endif // DIRECTIONAL_TECHNIQUE_H