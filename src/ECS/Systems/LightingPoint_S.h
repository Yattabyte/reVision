#pragma once
#ifndef LIGHTINGPOINT_S_H
#define LIGHTINGPOINT_S_H 

#include "ECS\Systems\ecsSystem.h"
#include "Assets\Asset_Shader.h"
#include "Assets\Asset_Primitive.h"
#include "ECS\Components\LightPoint_C.h"
#include "ECS\Resources\FBO_Shadow_Point.h"
#include "ECS\Systems\PropShadowing.h"
#include "Utilities\GL\StaticBuffer.h"
#include "Utilities\GL\DynamicBuffer.h"
#include "Utilities\GL\FBO.h"
#include "Utilities\PriorityList.h"
#include "Engine.h"
#include "GLFW\glfw3.h"
#include <vector>


/** A system that performs lighting operations for point lights. */
class LightingPoint_System : public BaseECSSystem {
public: 
	// (de)Constructors
	~LightingPoint_System() {
		m_engine->removePrefCallback(PreferenceState::C_WINDOW_WIDTH, this);
		m_engine->removePrefCallback(PreferenceState::C_WINDOW_HEIGHT, this);
		m_engine->removePrefCallback(PreferenceState::C_SHADOW_QUALITY, this);
		m_engine->removePrefCallback(PreferenceState::C_SHADOW_SIZE_POINT, this);
		if (m_shader_Lighting.get()) m_shader_Lighting->removeCallback(this);
		if (m_shapeSphere.get()) m_shapeSphere->removeCallback(this);
		glDeleteVertexArrays(1, &m_sphereVAO);
	}
	LightingPoint_System(
		Engine * engine, 
		FBO_Base * geometryFBO, FBO_Base * lightingFBO,
		GL_Vector * propBuffer, GL_Vector * skeletonBuffer
	) : BaseECSSystem(), m_engine(engine), m_geometryFBO(geometryFBO), m_lightingFBO(lightingFBO) {
		// Declare component types used
		addComponentType(LightPoint_Component::ID);
		addComponentType(LightPointShadow_Component::ID, FLAG_OPTIONAL);
		
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
		m_updateQuality = m_engine->addPrefCallback<unsigned int>(PreferenceState::C_SHADOW_QUALITY, this, [&](const float &f) {m_updateQuality = (unsigned int)f; });
		m_shadowSize.x = m_engine->addPrefCallback<int>(PreferenceState::C_SHADOW_SIZE_POINT, this, [&](const float &f) { m_shadowSize = glm::ivec2(std::max(1, (int)f)); });
		m_shadowSize = glm::ivec2(std::max(1, m_shadowSize.x));
		m_shader_Lighting->addCallback(this, [&](void) {m_shader_Lighting->setUniform(0, 1.0f / (float)m_shadowSize.x); });

		// Shadows
		m_shadowFBO.resize(m_shadowSize, 6);

		// Primitive Construction
		m_sphereVAOLoaded = false;
		m_sphereVAO = Asset_Primitive::Generate_VAO();
		m_indirectShape = StaticBuffer(sizeof(GLuint) * 4);
		m_shapeSphere->addCallback(this, [&]() mutable {
			m_sphereVAOLoaded = true;
			m_shapeSphere->updateVAO(m_sphereVAO);
			const GLuint data = { (GLuint)m_shapeSphere->getSize() };
			m_indirectShape.write(0, sizeof(GLuint), &data); // count, primCount, first, reserved
		});

		// Geometry rendering pipeline
		m_geometryStaticSystems.addSystem(new PropShadowing_System(m_engine, 6, PropShadowing_System::RenderStatic, m_shader_Culling, m_shader_Shadow, propBuffer, skeletonBuffer));
		m_geometryDynamicSystems.addSystem(new PropShadowing_System(m_engine, 6, PropShadowing_System::RenderDynamic, m_shader_Culling, m_shader_Shadow, propBuffer, skeletonBuffer));
		
		// Error Reporting
		const GLenum Status = glCheckNamedFramebufferStatus(m_shadowFBO.m_fboID, GL_FRAMEBUFFER);
		if (Status != GL_FRAMEBUFFER_COMPLETE && Status != GL_NO_ERROR)
			m_engine->reportError(MessageManager::FBO_INCOMPLETE, "Point Shadowmap FBO", std::string(reinterpret_cast<char const *>(glewGetErrorString(Status))));
	}


	// Interface Implementation	
	virtual void updateComponents(const float & deltaTime, const std::vector< std::vector<BaseECSComponent*> > & components) override {
		// Exit Early
		if (!m_sphereVAOLoaded || !m_shader_Lighting->existsYet() || !m_shader_Stencil->existsYet() || !m_shader_Shadow->existsYet())
			return;

		// Clear Data
		lightIndices.clear();
		shadowIndices.clear();
		m_oldest.clear();

		// Accumulate Light Data		
		for each (const auto & componentParam in components) {
			LightPoint_Component * lightComponent = (LightPoint_Component*)componentParam[0];
			LightPointShadow_Component * shadowComponent = (LightPointShadow_Component*)componentParam[1];
			lightIndices.push_back(lightComponent->m_data->index);
			if (shadowComponent) {
				shadowIndices.push_back(shadowComponent->m_data->index);
				m_oldest.insert(shadowComponent->m_updateTime, std::make_pair(lightComponent, shadowComponent));	
			}
			else
				shadowIndices.push_back(-1);
		}

		// Update Draw Buffers
		const size_t & lightSize = lightIndices.size();
		m_visLights.write(0, sizeof(GLuint) *lightSize, lightIndices.data());
		m_visShadows.write(0, sizeof(GLuint) *shadowIndices.size(), shadowIndices.data());
		m_indirectShape.write(sizeof(GLuint), sizeof(GLuint), &lightSize); // update primCount (2nd param)

		// Bind buffers common for rendering and shadowing
		m_lightBuffer.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 8);		// Light buffer
		m_shadowBuffer.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 9);	// Shadow buffer

		// Render important shadows
		renderShadows(deltaTime);
		// Render lights
		renderLights(deltaTime);
	}


	// Public Methods
	bool & outOfDate() { return m_outOfDate; }
	

	// Public Attributes
	VectorBuffer<LightPoint_Buffer> m_lightBuffer;
	VectorBuffer<LightPointShadow_Buffer> m_shadowBuffer;
	FBO_Shadow_Point m_shadowFBO;

	
protected:
	// Protected Methods
	/** Render all the geometry from each light */
	void renderShadows(const float & deltaTime) {
		ECS & ecs = m_engine->getECS();
		glViewport(0, 0, m_shadowSize.x, m_shadowSize.y);
		m_shader_Shadow->bind();
		m_shadowFBO.bindForWriting();
		for each (const auto & pair in PQtoVector()) {
			glUniform1i(0, pair.first->m_data->index);
			glUniform1i(1, pair.second->m_data->index);
			if (pair.second->m_outOfDate || m_outOfDate) {
				// update static shadows
				m_shadowFBO.clear(pair.second->m_shadowSpot + 6);
				ecs.updateSystems(m_geometryStaticSystems, deltaTime);
				pair.second->m_outOfDate = false;
			}
			// update dynamic shadows
			m_shadowFBO.clear(pair.second->m_shadowSpot);
			ecs.updateSystems(m_geometryDynamicSystems, deltaTime);
			pair.second->m_updateTime = (float)glfwGetTime();
		}

		if (m_outOfDate)
			m_outOfDate = false;
		glViewport(0, 0, m_renderSize.x, m_renderSize.y);
	}
	/** Render all the lights */
	void renderLights(const float & deltaTime) {
		glEnable(GL_STENCIL_TEST);
		glEnable(GL_BLEND);
		glBlendEquation(GL_FUNC_ADD);
		glBlendFunc(GL_ONE, GL_ONE);

		// Draw only into depth-stencil buffer
		m_shader_Stencil->bind();										// Shader (point)
		m_lightingFBO->bindForWriting();								// Ensure writing to lighting FBO
		m_geometryFBO->bindForReading();								// Read from Geometry FBO
		glBindTextureUnit(4, m_shadowFBO.m_textureIDS[0]);				// Shadow map(linear depth texture)
		m_visLights.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 3);		// SSBO visible light indices
		m_visShadows.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 4);		// SSBO visible shadow indices
		m_indirectShape.bindBuffer(GL_DRAW_INDIRECT_BUFFER);			// Draw call buffer
		glBindVertexArray(m_sphereVAO);									// Quad VAO
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
	// Private methods
	const std::vector<std::pair<LightPoint_Component*, LightPointShadow_Component*>> PQtoVector() const {
		PriorityList<float, std::pair<LightPoint_Component*, LightPointShadow_Component*>, std::greater<float>> m_closest(m_updateQuality / 2);
		std::vector<std::pair<LightPoint_Component*, LightPointShadow_Component*>> outList;
		outList.reserve(m_updateQuality);

		for each (const auto &element in m_oldest.toList()) {
			if (outList.size() < (m_updateQuality / 2))
				outList.push_back(element);
			else
				m_closest.insert(element.second->m_updateTime, element);
		}

		for each (const auto &element in m_closest.toList()) {
			if (outList.size() >= m_updateQuality)
				break;
			outList.push_back(element);
		}

		return outList;
	}


	// Private Attributes
	Engine * m_engine;
	Shared_Asset_Shader m_shader_Lighting, m_shader_Stencil, m_shader_Shadow, m_shader_Culling;
	Shared_Asset_Primitive m_shapeSphere;
	GLuint m_sphereVAO;
	bool m_sphereVAOLoaded;
	unsigned int m_updateQuality;
	glm::ivec2 m_shadowSize;
	glm::ivec2	m_renderSize;
	StaticBuffer m_indirectShape;
	std::vector<GLint> lightIndices, shadowIndices;
	DynamicBuffer m_visLights, m_visShadows;
	PriorityList<float, std::pair<LightPoint_Component*, LightPointShadow_Component*>, std::less<float>> m_oldest;
	ECSSystemList m_geometryStaticSystems, m_geometryDynamicSystems;
	FBO_Base * m_geometryFBO, * m_lightingFBO;
	bool m_outOfDate = true;
};

#endif // LIGHTINGPOINT_S_H