#pragma once
#ifndef LIGHTPOINT_FX_H
#define LIGHTPOINT_FX_H 

#include "Modules\Graphics\Effects\Effect_Base.h"
#include "Assets\Asset_Shader.h"
#include "Assets\Asset_Primitive.h"
#include "ECS\Systems\LightPoint_S.h"
#include "ECS\Systems\PropShadowing_S.h"
#include "Modules\Graphics\Common\FBO_Shadow_Point.h"
#include "Modules\Graphics\Effects\PropShadowing_FX.h"
#include "Utilities\GL\FBO.h"
#include "Engine.h"
#include "GLFW\glfw3.h"


/** A core rendering effect which applies point lighting to the scene. */
class LightPoint_Effect : public Effect_Base {
public:
	// (de)Constructors
	/** Virtual Destructor. */
	~LightPoint_Effect() {
		m_engine->removePrefCallback(PreferenceState::C_WINDOW_WIDTH, this);
		m_engine->removePrefCallback(PreferenceState::C_WINDOW_HEIGHT, this);
		m_engine->removePrefCallback(PreferenceState::C_SHADOW_QUALITY, this);
		m_engine->removePrefCallback(PreferenceState::C_SHADOW_SIZE_POINT, this);
		if (m_shader_Lighting.get()) m_shader_Lighting->removeCallback(this);
		if (m_shapeSphere.get()) m_shapeSphere->removeCallback(this);
	}
	/** Constructor. */
	LightPoint_Effect(
		Engine * engine,
		FBO_Base * geometryFBO, FBO_Base * lightingFBO,
		GL_Vector * propBuffer, GL_Vector * skeletonBuffer,
		Point_RenderState * renderState
	) : m_engine(engine), m_geometryFBO(geometryFBO), m_lightingFBO(lightingFBO), m_renderState(renderState) {
		// Asset Loading
		m_shader_Lighting = Asset_Shader::Create(m_engine, "Core\\Point\\Light");
		m_shader_Stencil = Asset_Shader::Create(m_engine, "Core\\Point\\Stencil");
		m_shader_Shadow = Asset_Shader::Create(m_engine, "Core\\Point\\Shadow");
		m_shader_Culling = Asset_Shader::Create(m_engine, "Core\\Point\\Culling");
		m_shapeSphere = Asset_Primitive::Create(m_engine, "sphere");

		// Preference Callbacks
		m_renderSize.x = m_engine->addPrefCallback<int>(PreferenceState::C_WINDOW_WIDTH, this, [&](const float &f) {
			m_renderSize = glm::ivec2(f, m_renderSize.y);
		});
		m_renderSize.y = m_engine->addPrefCallback<int>(PreferenceState::C_WINDOW_HEIGHT, this, [&](const float &f) {
			m_renderSize = glm::ivec2(m_renderSize.x, f);
		});
		m_renderState->m_updateQuality = m_engine->addPrefCallback<unsigned int>(PreferenceState::C_SHADOW_QUALITY, this, [&](const float &f) { m_renderState->m_updateQuality = (unsigned int)f; });
		m_renderState->m_shadowSize.x = m_engine->addPrefCallback<int>(PreferenceState::C_SHADOW_SIZE_POINT, this, [&](const float &f) { m_renderState->m_shadowSize = glm::ivec2(std::max(1, (int)f)); });
		m_renderState->m_shadowSize = glm::ivec2(std::max(1, m_renderState->m_shadowSize.x));
		m_shadowFBO.resize(m_renderState->m_shadowSize, 6);

		// Asset-Finished Callbacks
		m_shapeSphere->addCallback(this, [&]() mutable {
			const GLuint data = { (GLuint)m_shapeSphere->getSize() };
			m_renderState->m_indirectShape.write(0, sizeof(GLuint), &data); // count, primCount, first, reserved
		});
		m_shader_Lighting->addCallback(this, [&](void) {m_shader_Lighting->setUniform(0, 1.0f / (float)m_renderState->m_shadowSize.x); });

		// Geometry rendering pipeline
		m_geometryStaticSystems.addSystem(new PropShadowing_System(m_engine, 6, PropShadowing_System::RenderStatic));
		m_geometryDynamicSystems.addSystem(new PropShadowing_System(m_engine, 6, PropShadowing_System::RenderDynamic));
		m_geometryEffectsStatic.push_back(new PropShadowing_Effect(engine, m_shader_Culling, m_shader_Shadow, propBuffer, skeletonBuffer, &((PropShadowing_System*)m_geometryStaticSystems[0])->m_renderState));
		m_geometryEffectsDynamic.push_back(new PropShadowing_Effect(engine, m_shader_Culling, m_shader_Shadow, propBuffer, skeletonBuffer, &((PropShadowing_System*)m_geometryDynamicSystems[0])->m_renderState));

		// Error Reporting
		const GLenum Status = glCheckNamedFramebufferStatus(m_shadowFBO.m_fboID, GL_FRAMEBUFFER);
		if (Status != GL_FRAMEBUFFER_COMPLETE && Status != GL_NO_ERROR)
			m_engine->reportError(MessageManager::FBO_INCOMPLETE, "Point Shadowmap FBO", std::string(reinterpret_cast<char const *>(glewGetErrorString(Status))));
	}


	// Interface Implementations.
	virtual void applyEffect(const float & deltaTime) override {
		// Exit Early
		if (!m_shapeSphere->existsYet() || !m_shader_Lighting->existsYet() || !m_shader_Stencil->existsYet() || !m_shader_Shadow->existsYet() || !m_shader_Culling->existsYet())
			return;

		// Bind buffers common for rendering and shadowing
		m_lightBuffer.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 8); // Light buffer
		m_shadowBuffer.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 9); // Shadow buffer

		// Render important shadows
		renderShadows(deltaTime);
		// Render direct lights
		renderLights(deltaTime);
	}


	// Public Attributes
	VectorBuffer<LightPoint_Buffer> m_lightBuffer;
	VectorBuffer<LightPointShadow_Buffer> m_shadowBuffer;
	FBO_Shadow_Point m_shadowFBO;


protected:
	// Protected Methods
	/** Render all the geometry from each light. */
	void renderShadows(const float & deltaTime) {
		ECS & ecs = m_engine->getECS();
		glViewport(0, 0, m_renderState->m_shadowSize.x, m_renderState->m_shadowSize.y);
		m_shader_Shadow->bind();
		m_shadowFBO.bindForWriting();
		for each (const auto & pair in m_renderState->m_shadowsToUpdate) {
			glUniform1i(0, pair.first->m_data->index);
			glUniform1i(1, pair.second->m_data->index);
			// Update static shadows
			if (pair.second->m_outOfDate || m_renderState->m_outOfDate) {
				m_shadowFBO.clear(pair.second->m_shadowSpot + 6);
				ecs.updateSystems(m_geometryStaticSystems, deltaTime);
				for each (auto *tech in m_geometryEffectsStatic)
					if (tech->isEnabled())
						tech->applyEffect(deltaTime);
				pair.second->m_outOfDate = false;
			}
			// Update dynamic shadows
			m_shadowFBO.clear(pair.second->m_shadowSpot);
			ecs.updateSystems(m_geometryDynamicSystems, deltaTime);
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
	void renderLights(const float & deltaTime) {
		glEnable(GL_STENCIL_TEST);
		glEnable(GL_BLEND);
		glBlendEquation(GL_FUNC_ADD);
		glBlendFunc(GL_ONE, GL_ONE);

		// Draw only into depth-stencil buffer
		m_shader_Stencil->bind();													// Shader (point)
		m_lightingFBO->bindForWriting();											// Ensure writing to lighting FBO
		m_geometryFBO->bindForReading();											// Read from Geometry FBO
		glBindTextureUnit(4, m_shadowFBO.m_textureIDS[0]);							// Shadow map(linear depth texture)
		m_renderState->m_visLights.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 3);		// SSBO visible light indices
		m_renderState->m_visShadows.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 4);	// SSBO visible shadow indices
		m_renderState->m_indirectShape.bindBuffer(GL_DRAW_INDIRECT_BUFFER);			// Draw call buffer
		glBindVertexArray(m_shapeSphere->m_vaoID);									// Quad VAO
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
	Engine * m_engine;
	glm::ivec2 m_renderSize = glm::ivec2(1);
	Shared_Asset_Shader m_shader_Lighting, m_shader_Stencil, m_shader_Shadow, m_shader_Culling;
	Shared_Asset_Primitive m_shapeSphere;	
	ECSSystemList m_geometryStaticSystems, m_geometryDynamicSystems;
	std::vector<Effect_Base*> m_geometryEffectsStatic, m_geometryEffectsDynamic;
	FBO_Base * m_geometryFBO, * m_lightingFBO;
	Point_RenderState * m_renderState;
};

#endif // LIGHTPOINT_FX_H