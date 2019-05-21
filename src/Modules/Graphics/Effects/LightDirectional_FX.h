#pragma once
#ifndef LIGHTDIRECTIONAL_FX_H
#define LIGHTDIRECTIONAL_FX_H

#include "Modules/Graphics/Effects/GFX_Core_Effect.h"
#include "Assets/Shader.h"
#include "Assets/Primitive.h"
#include "Modules/Graphics/ECS/LightDirectional_S.h"
#include "Modules/Graphics/ECS/PropShadowing_S.h"
#include "Modules/Graphics/Common/FBO_Shadow_Directional.h"
#include "Modules/Graphics/Common/RH_Volume.h"
#include "Modules/Graphics/Effects/PropShadowing_FX.h"
#include "Utilities/GL/FBO.h"
#include "Engine.h"
#include "GLFW/glfw3.h"
#include <random>


/** A core-rendering technique which applies directional lighting to the scene. */
class LightDirectional_Effect : public GFX_Core_Effect {
public:
	// (de)Constructors
	/** Virtual Destructor. */
	~LightDirectional_Effect() {
		// Update indicator
		m_aliveIndicator = false;
	}
	/** Constructor. */
	LightDirectional_Effect(
		Engine * engine,
		FBO_Base * geometryFBO, FBO_Base * lightingFBO, FBO_Base * bounceFBO,
		GL_Vector * propBuffer, GL_Vector * skeletonBuffer,
		Directional_RenderState * renderState,
		std::shared_ptr<RH_Volume> volumeRH
	) : m_engine(engine), m_geometryFBO(geometryFBO), m_lightingFBO(lightingFBO), m_bounceFBO(bounceFBO), m_renderState(renderState), m_volumeRH(volumeRH) {
		// Asset Loading
		m_shader_Lighting = Shared_Shader(m_engine, "Core\\Directional\\Light");
		m_shader_Shadow = Shared_Shader(m_engine, "Core\\Directional\\Shadow");
		m_shader_Culling = Shared_Shader(m_engine, "Core\\Directional\\Culling");
		m_shader_Bounce = Shared_Shader(m_engine, "Core\\Directional\\Bounce");
		m_shapeQuad = Shared_Primitive(engine, "quad");

		// Preferences
		auto & preferences = m_engine->getPreferenceState();
		preferences.getOrSetValue(PreferenceState::C_WINDOW_WIDTH, m_renderSize.x);
		preferences.addCallback(PreferenceState::C_WINDOW_WIDTH, m_aliveIndicator, [&](const float &f) {m_renderSize = glm::ivec2(f, m_renderSize.y); });
		preferences.getOrSetValue(PreferenceState::C_WINDOW_HEIGHT, m_renderSize.y);
		preferences.addCallback(PreferenceState::C_WINDOW_HEIGHT, m_aliveIndicator, [&](const float &f) {m_renderSize = glm::ivec2(m_renderSize.x, f); });
		preferences.getOrSetValue(PreferenceState::C_DRAW_DISTANCE, m_renderState->m_drawDistance);
		preferences.addCallback(PreferenceState::C_DRAW_DISTANCE, m_aliveIndicator, [&](const float &f) { m_renderState->m_drawDistance = f; });
		preferences.getOrSetValue(PreferenceState::C_SHADOW_QUALITY, m_renderState->m_updateQuality);
		preferences.addCallback(PreferenceState::C_SHADOW_QUALITY, m_aliveIndicator, [&](const float &f) { m_renderState->m_updateQuality = (unsigned int)f; });
		preferences.getOrSetValue(PreferenceState::C_SHADOW_SIZE_DIRECTIONAL, m_renderState->m_shadowSize.x);
		preferences.addCallback(PreferenceState::C_SHADOW_SIZE_DIRECTIONAL, m_aliveIndicator, [&](const float &f) { m_renderState->m_shadowSize = glm::ivec2(std::max(1, (int)f)); });
		preferences.getOrSetValue(PreferenceState::C_RH_BOUNCE_SIZE, m_renderState->m_bounceSize);
		preferences.addCallback(PreferenceState::C_RH_BOUNCE_SIZE, m_aliveIndicator, [&](const float &f) { m_renderState->m_bounceSize = (GLuint)f; });
		m_renderState->m_shadowSize = glm::ivec2(std::max(1, m_renderState->m_shadowSize.x));
		m_shadowFBO.resize(m_renderState->m_shadowSize, 4);

		// Asset-Finished Callbacks
		m_shapeQuad->addCallback(m_aliveIndicator, [&]() mutable {
			// count, primCount, first, reserved
			const GLuint data[4] = { (GLuint)m_shapeQuad->getSize(), 0, 0, 0 };
			m_renderState->m_indirectShape.write(0, sizeof(GLuint) * 4, &data);
			m_renderState->m_indirectBounce.write(0, sizeof(GLuint) * 4, &data);
		});
		m_shader_Lighting->addCallback(m_aliveIndicator, [&](void) {m_shader_Lighting->setUniform(0, 1.0f / m_renderState->m_shadowSize.x); });

		// Noise Texture
		std::uniform_real_distribution<float> randomFloats(0.0, 1.0);
		std::default_random_engine generator;
		glm::vec3 data[32 * 32 * 32];
		for (int x = 0, total = (32 * 32 * 32); x < total; ++x)
			data[x] = glm::vec3(randomFloats(generator), randomFloats(generator), randomFloats(generator));
		glCreateTextures(GL_TEXTURE_3D, 1, &m_textureNoise32);
		glTextureImage3DEXT(m_textureNoise32, GL_TEXTURE_3D, 0, GL_RGB16F, 32, 32, 32, 0, GL_RGB, GL_FLOAT, &data);
		glTextureParameteri(m_textureNoise32, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
		glTextureParameteri(m_textureNoise32, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
		glTextureParameteri(m_textureNoise32, GL_TEXTURE_WRAP_R, GL_MIRRORED_REPEAT);
		glTextureParameteri(m_textureNoise32, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTextureParameteri(m_textureNoise32, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

		// Geometry rendering pipeline
		m_geometrySystems.addSystem(new PropShadowing_System(engine, 4, PropShadowing_System::RenderAll));
		m_geometryEffects.push_back(new PropShadowing_Effect(engine, m_shader_Culling, m_shader_Shadow, propBuffer, skeletonBuffer, &((PropShadowing_System*)m_geometrySystems[0])->m_renderState));

		// Error Reporting
		if (glCheckNamedFramebufferStatus(m_shadowFBO.m_fboID, GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			m_engine->getManager_Messages().error("DirectionalLight Shadowmap Framebuffer has encountered an error.");		
	}


	// Interface Implementations.
	inline virtual void applyEffect(const float & deltaTime) override {
		// Exit Early
		if (!m_shapeQuad->existsYet() || !m_shader_Lighting->existsYet() || !m_shader_Shadow->existsYet() || !m_shader_Culling->existsYet() || !m_shader_Bounce->existsYet())
			return;

		// Bind buffers common for rendering and shadowing
		m_lightBuffer.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 8); // Light buffer
		m_shadowBuffer.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 9); // Shadow buffer

		// Render important shadows
		renderShadows(deltaTime);
		// Render lights
		renderLights(deltaTime);
		// Render indirect lights
		renderBounce(deltaTime);
	}
	

	// Public Attributes
	VectorBuffer<LightDirectional_Buffer> m_lightBuffer;
	VectorBuffer<LightDirectionalShadow_Buffer> m_shadowBuffer;
	FBO_Shadow_Directional m_shadowFBO;


private:
	// Private Methods
	/** Render all the geometry from each light. */
	inline void renderShadows(const float & deltaTime) {
		ECS & ecs = m_engine->getECS();
		glViewport(0, 0, m_renderState->m_shadowSize.x, m_renderState->m_shadowSize.y);
		m_shader_Shadow->bind();
		m_shadowFBO.bindForWriting();

		for each (const auto & pair in m_renderState->m_shadowsToUpdate) {
			const float clearDepth(1.0f);
			const glm::vec3 clear(0.0f);
			glUniform1i(0, pair.first->m_data->index);
			glUniform1i(1, pair.second->m_data->index);
			m_shadowFBO.clear(pair.second->m_shadowSpot);
			// Update geometry components
			ecs.updateSystems(m_geometrySystems, deltaTime);
			// Render geometry components
			for each (auto *tech in m_geometryEffects)
				if (tech->isEnabled())
					tech->applyEffect(deltaTime);
			pair.second->m_updateTime = (float)glfwGetTime();
		}

		glViewport(0, 0, m_renderSize.x, m_renderSize.y);
	}
	/** Render all the lights. */
	inline void renderLights(const float & deltaTime) {
		glEnable(GL_BLEND);
		glBlendEquation(GL_FUNC_ADD);
		glBlendFunc(GL_ONE, GL_ONE);
		glDisable(GL_DEPTH_TEST);
		glDepthMask(GL_FALSE);

		m_shader_Lighting->bind();													// Shader (directional)
		m_lightingFBO->bindForWriting();											// Ensure writing to lighting FBO
		m_geometryFBO->bindForReading();											// Read from Geometry FBO
		glBindTextureUnit(4, m_shadowFBO.m_textureIDS[2]);							// Shadow map (depth texture)
		m_renderState->m_visLights.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 3);		// SSBO visible light indices
		m_renderState->m_visShadows.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 4);	// SSBO visible shadow indices
		m_renderState->m_indirectShape.bindBuffer(GL_DRAW_INDIRECT_BUFFER);			// Draw call buffer
		glBindVertexArray(m_shapeQuad->m_vaoID);									// Quad VAO
		glDrawArraysIndirect(GL_TRIANGLES, 0);										// Now draw

		glDepthMask(GL_TRUE);
		glEnable(GL_DEPTH_TEST);
		glBlendFunc(GL_ONE, GL_ZERO);
		glDisable(GL_BLEND);
	}
	/** Render light bounces. */
	inline void renderBounce(const float & deltaTime) {
		m_shader_Bounce->setUniform(0, m_renderState->m_shadowCount);
		m_shader_Bounce->setUniform(1, m_volumeRH->m_max);
		m_shader_Bounce->setUniform(2, m_volumeRH->m_min);
		m_shader_Bounce->setUniform(4, m_volumeRH->m_resolution);
		m_shader_Bounce->setUniform(6, m_volumeRH->m_unitSize);		

		// Prepare rendering state
		glDisable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);
		glBlendEquation(GL_FUNC_ADD);
		glBlendFunc(GL_ONE, GL_ONE);
		glBlendEquationSeparatei(0, GL_MIN, GL_MIN);
		glBindVertexArray(m_shapeQuad->m_vaoID);

		glViewport(0, 0, (GLsizei)m_volumeRH->m_resolution, (GLsizei)m_volumeRH->m_resolution);
		m_bounceFBO->bindForWriting();
		m_shader_Bounce->bind();
		m_geometryFBO->bindForReading(0); // depth -> 3		
		glBindTextureUnit(0, m_shadowFBO.m_textureIDS[0]);
		glBindTextureUnit(1, m_shadowFBO.m_textureIDS[1]);
		glBindTextureUnit(2, m_shadowFBO.m_textureIDS[2]);
		glBindTextureUnit(4, m_textureNoise32);
		m_renderState->m_visLights.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 3);		// SSBO visible light indices
		m_renderState->m_visShadows.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 4);		// SSBO visible shadow indices
		m_renderState->m_indirectBounce.bindBuffer(GL_DRAW_INDIRECT_BUFFER);
		glDrawArraysIndirect(GL_TRIANGLES, 0);
		glViewport(0, 0, m_renderSize.x, m_renderSize.y);
	}


	// Private Attributes
	Engine * m_engine = nullptr;
	Shared_Shader m_shader_Lighting, m_shader_Shadow, m_shader_Culling, m_shader_Bounce;
	Shared_Primitive m_shapeQuad;	
	glm::ivec2 m_renderSize = glm::ivec2(1);
	ECSSystemList m_geometrySystems;
	std::vector<GFX_Core_Effect*> m_geometryEffects;
	FBO_Base * m_geometryFBO = nullptr, * m_lightingFBO = nullptr, * m_bounceFBO = nullptr;
	GLuint m_textureNoise32 = 0;
	Directional_RenderState * m_renderState = nullptr;
	std::shared_ptr<RH_Volume> m_volumeRH;
	std::shared_ptr<bool> m_aliveIndicator = std::make_shared<bool>(true);
};

#endif // LIGHTDIRECTIONAL_FX_H