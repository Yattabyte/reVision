#pragma once
#ifndef POINT_LIGHTING_H
#define POINT_LIGHTING_H

#include "Modules/Graphics/Graphics_Technique.h"
#include "Modules/Graphics/Lighting/components.h"
#include "Modules/Graphics/Geometry/Prop_Shadow.h"
#include "Modules/Graphics/Common/FBO_Shadow_Point.h"
#include "Modules/World/ECS/ecsSystem.h"
#include "Assets/Shader.h"
#include "Assets/Primitive.h"
#include "Utilities/GL/StaticBuffer.h"
#include "Utilities/GL/DynamicBuffer.h"
#include "Utilities/GL/FBO.h"
#include "Utilities/PriorityList.h"
#include "Engine.h"
#include <vector>


/***/
class Point_Lighting : public Graphics_Technique, public BaseECSSystem {
public:
	// Public (de)Constructors
	/** Destructor. */
	inline ~Point_Lighting() {
		// Update indicator
		m_aliveIndicator = false;
		auto & world = m_engine->getModule_World();
		world.removeComponentType("LightPoint_Component");
		world.removeComponentType("LightPointShadow_Component");
	}
	/** Constructor. */
	inline Point_Lighting(Engine * engine, FBO_Base * geometryFBO, FBO_Base * lightingFBO, Prop_View * propView)
		: m_engine(engine), m_geometryFBO(geometryFBO), m_lightingFBO(lightingFBO) {
		// Asset Loading
		m_shader_Lighting = Shared_Shader(m_engine, "Core\\Point\\Light");
		m_shader_Stencil = Shared_Shader(m_engine, "Core\\Point\\Stencil");
		m_shader_Shadow = Shared_Shader(m_engine, "Core\\Point\\Shadow");
		m_shader_Culling = Shared_Shader(m_engine, "Core\\Point\\Culling");
		m_shapeSphere = Shared_Primitive(m_engine, "sphere");

		// Preferences
		auto & preferences = m_engine->getPreferenceState();
		preferences.getOrSetValue(PreferenceState::C_WINDOW_WIDTH, m_renderSize.x);
		preferences.addCallback(PreferenceState::C_WINDOW_WIDTH, m_aliveIndicator, [&](const float &f) { m_renderSize = glm::ivec2(f, m_renderSize.y); });
		preferences.getOrSetValue(PreferenceState::C_WINDOW_HEIGHT, m_renderSize.y);
		preferences.addCallback(PreferenceState::C_WINDOW_HEIGHT, m_aliveIndicator, [&](const float &f) { m_renderSize = glm::ivec2(m_renderSize.x, f); });
		preferences.getOrSetValue(PreferenceState::C_SHADOW_QUALITY, m_updateQuality);
		preferences.addCallback(PreferenceState::C_SHADOW_QUALITY, m_aliveIndicator, [&](const float &f) { m_updateQuality = (unsigned int)f; });
		preferences.getOrSetValue(PreferenceState::C_SHADOW_SIZE_POINT, m_shadowSize.x);
		preferences.addCallback(PreferenceState::C_SHADOW_SIZE_POINT, m_aliveIndicator, [&](const float &f) {
			m_shadowSize = glm::ivec2(std::max(1, (int)f));
			m_shadowFBO.resize(m_shadowSize, 6);
			if (m_shader_Lighting && m_shader_Lighting->existsYet())
				m_shader_Lighting->addCallback(m_aliveIndicator, [&](void) { m_shader_Lighting->setUniform(0, 1.0f / (float)m_shadowSize.x); });
		});
		m_shadowSize = glm::ivec2(std::max(1, m_shadowSize.x));
		m_shadowFBO.resize(m_shadowSize, 6);

		// Asset-Finished Callbacks
		m_shapeSphere->addCallback(m_aliveIndicator, [&]() mutable {
			const GLuint data = { (GLuint)m_shapeSphere->getSize() };
			m_indirectShape.write(0, sizeof(GLuint), &data); // count, primCount, first, reserved
		});
		m_shader_Lighting->addCallback(m_aliveIndicator, [&](void) { m_shader_Lighting->setUniform(0, 1.0f / (float)m_shadowSize.x); });

		// Geometry rendering pipeline
		m_propShadow_Static = new Prop_Shadow(m_engine, 6, Prop_Shadow::RenderStatic, m_shader_Culling, m_shader_Shadow, propView);
		m_propShadow_Dynamic = new Prop_Shadow(m_engine, 6, Prop_Shadow::RenderDynamic, m_shader_Culling, m_shader_Shadow, propView);
		
		// Error Reporting
		if (glCheckNamedFramebufferStatus(m_shadowFBO.m_fboID, GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			m_engine->getManager_Messages().error("Point_Lighting Shadowmap Framebuffer has encountered an error.");
		
		// Declare component types used
		addComponentType(LightPoint_Component::ID);
		addComponentType(LightPointShadow_Component::ID, FLAG_OPTIONAL);
		GLuint data[] = { 0,0,0,0 };
		m_indirectShape.write(0, sizeof(GLuint) * 4, &data);

		// Error Reporting
		if (!isValid())
			engine->getManager_Messages().error("Invalid ECS System: Point_Lighting");

		// Add New Component Types
		auto & world = m_engine->getModule_World();
		world.addLevelListener(&m_outOfDate);
		world.addComponentType("LightPoint_Component", [&, engine](const ParamList & parameters) {
			auto color = CastAny(parameters[0], glm::vec3(1.0f));
			auto intensity = CastAny(parameters[1], 1.0f);
			auto radius = CastAny(parameters[2], 1.0f);
			auto * component = new LightPoint_Component();
			component->m_data = m_lightBuffer.newElement();
			component->m_data->data->LightColor = color;
			component->m_data->data->LightIntensity = intensity;
			component->m_data->data->LightRadius = radius;
			component->m_radius = radius;
			return std::make_pair(component->ID, component);
		});
		world.addComponentType("LightPointShadow_Component", [&, engine](const ParamList & parameters) {
			auto shadowSpot = (int)(m_shadowBuffer.getCount() * 12);
			auto * component = new LightPointShadow_Component();
			component->m_radius = CastAny(parameters[0], 1.0f);
			component->m_data = m_shadowBuffer.newElement();
			component->m_data->data->Shadow_Spot = shadowSpot;
			component->m_updateTime = 0.0f;
			component->m_shadowSpot = shadowSpot;
			component->m_outOfDate = true;
			m_shadowFBO.resize(m_shadowFBO.m_size, shadowSpot + 12);
			return std::make_pair(component->ID, component);
		});
	}


	// Public Interface Implementations
	inline virtual void applyEffect(const float & deltaTime) override {
		// Exit Early
		if (!m_enabled || !m_shapeSphere->existsYet() || !m_shader_Lighting->existsYet() || !m_shader_Stencil->existsYet() || !m_shader_Shadow->existsYet() || !m_shader_Culling->existsYet())
			return;

		// Bind buffers common for rendering and shadowing
		m_lightBuffer.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 8); // Light buffer
		m_shadowBuffer.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 9); // Shadow buffer

		// Render important shadows
		renderShadows(deltaTime);
		// Render direct lights
		renderLights(deltaTime);
	}
	inline virtual void updateComponents(const float & deltaTime, const std::vector< std::vector<BaseECSComponent*> > & components) override {
		// Accumulate Light Data	
		std::vector<GLint> lightIndices, shadowIndices;
		PriorityList<float, std::pair<LightPoint_Component*, LightPointShadow_Component*>, std::less<float>> oldest;
		for each (const auto & componentParam in components) {
			LightPoint_Component * lightComponent = (LightPoint_Component*)componentParam[0];
			LightPointShadow_Component * shadowComponent = (LightPointShadow_Component*)componentParam[1];
			lightIndices.push_back(lightComponent->m_data->index);
			if (shadowComponent) {
				shadowIndices.push_back(shadowComponent->m_data->index);
				oldest.insert(shadowComponent->m_updateTime, std::make_pair(lightComponent, shadowComponent));
			}
			else
				shadowIndices.push_back(-1);
		}

		// Update Draw Buffers
		const size_t & lightSize = lightIndices.size();
		m_visLights.write(0, sizeof(GLuint) * lightSize, lightIndices.data());
		m_visShadows.write(0, sizeof(GLuint) * shadowIndices.size(), shadowIndices.data());
		m_indirectShape.write(sizeof(GLuint), sizeof(GLuint), &lightSize); // update primCount (2nd param)		
		m_shadowsToUpdate = PQtoVector(oldest);
	}


private:
	// Protected Methods
	/** Render all the geometry from each light. */
	inline void renderShadows(const float & deltaTime) {
		auto & world = m_engine->getModule_World();
		glViewport(0, 0, m_shadowSize.x, m_shadowSize.y);
		m_shader_Shadow->bind();
		m_shadowFBO.bindForWriting();
		for each (const auto & pair in m_shadowsToUpdate) {
			glUniform1i(0, pair.first->m_data->index);
			glUniform1i(1, pair.second->m_data->index);
			// Update static shadows
			if (pair.second->m_outOfDate || m_outOfDate) {
				m_shadowFBO.clear(pair.second->m_shadowSpot + 6);
				// Update components
				world.updateSystem(m_propShadow_Static, deltaTime);
				// Render components
				m_propShadow_Static->applyEffect(deltaTime);
				pair.second->m_outOfDate = false;
			}
			// Update dynamic shadows
			m_shadowFBO.clear(pair.second->m_shadowSpot);
			// Update components
			world.updateSystem(m_propShadow_Dynamic, deltaTime);
			// Render components
			m_propShadow_Dynamic->applyEffect(deltaTime);
			pair.second->m_updateTime = m_engine->getTime();
		}

		if (m_outOfDate)
			m_outOfDate = false;
		glViewport(0, 0, m_renderSize.x, m_renderSize.y);
	}
	/** Render all the lights. */
	inline void renderLights(const float & deltaTime) {
		glEnable(GL_STENCIL_TEST);
		glEnable(GL_BLEND);
		glBlendEquation(GL_FUNC_ADD);
		glBlendFunc(GL_ONE, GL_ONE);

		// Draw only into depth-stencil buffer
		m_shader_Stencil->bind();									// Shader (point)
		m_lightingFBO->bindForWriting();							// Ensure writing to lighting FBO
		m_geometryFBO->bindForReading();							// Read from Geometry FBO
		glBindTextureUnit(4, m_shadowFBO.m_textureIDS[0]);			// Shadow map(linear depth texture)
		m_visLights.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 3);	// SSBO visible light indices
		m_visShadows.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 4);	// SSBO visible shadow indices
		m_indirectShape.bindBuffer(GL_DRAW_INDIRECT_BUFFER);		// Draw call buffer
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

		glCullFace(GL_BACK);
		glDepthMask(GL_TRUE);
		glBlendFunc(GL_ONE, GL_ZERO);
		glDisable(GL_BLEND);
		glDisable(GL_STENCIL_TEST);
	}
	/** Converts a priority queue into an stl vector.*/
	inline std::vector<std::pair<LightPoint_Component*, LightPointShadow_Component*>> PQtoVector(PriorityList<float, std::pair<LightPoint_Component*, LightPointShadow_Component*>, std::less<float>> oldest) const {
		PriorityList<float, std::pair<LightPoint_Component*, LightPointShadow_Component*>, std::greater<float>> m_closest(m_updateQuality / 2);
		std::vector<std::pair<LightPoint_Component*, LightPointShadow_Component*>> outList;
		outList.reserve(m_updateQuality);
		for each (const auto &element in oldest.toList()) {
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
	Engine * m_engine = nullptr;
	glm::ivec2 m_renderSize = glm::ivec2(1);
	Shared_Shader m_shader_Lighting, m_shader_Stencil, m_shader_Shadow, m_shader_Culling;
	Shared_Primitive m_shapeSphere;
	Prop_Shadow * m_propShadow_Static = nullptr, * m_propShadow_Dynamic = nullptr;
	FBO_Base * m_geometryFBO = nullptr, *m_lightingFBO = nullptr;
	VectorBuffer<LightPoint_Component::GL_Buffer> m_lightBuffer;
	VectorBuffer<LightPointShadow_Component::GL_Buffer> m_shadowBuffer;
	FBO_Shadow_Point m_shadowFBO;
	GLuint m_updateQuality = 1u;
	glm::ivec2 m_shadowSize = glm::ivec2(512);
	StaticBuffer m_indirectShape = StaticBuffer(sizeof(GLuint) * 4);
	DynamicBuffer m_visLights, m_visShadows;
	std::vector<std::pair<LightPoint_Component*, LightPointShadow_Component*>> m_shadowsToUpdate;
	bool m_outOfDate = true;
	std::shared_ptr<bool> m_aliveIndicator = std::make_shared<bool>(true);
};

#endif // DIRECTIONAL_LIGHTING_H