#pragma once
#ifndef POINT_LIGHTING_H
#define POINT_LIGHTING_H

#include "Modules/Graphics/Common/Graphics_Technique.h"
#include "Modules/Graphics/Lighting/components.h"
#include "Modules/Graphics/Lighting/FBO_Shadow_Point.h"
#include "Modules/Graphics/Geometry/Prop_Shadow.h"
#include "Modules/World/ECS/TransformComponent.h"
#include "Assets/Shader.h"
#include "Assets/Primitive.h"
#include "Utilities/GL/StaticBuffer.h"
#include "Utilities/GL/DynamicBuffer.h"
#include "Utilities/GL/GL_ArrayBuffer.h"
#include "Utilities/PriorityList.h"
#include "Engine.h"
#include <vector>


/** A core lighting technique responsible for all point lights. */
class Point_Lighting : public Graphics_Technique {
public:
	// Public (de)Constructors
	/** Destructor. */
	inline ~Point_Lighting() {
		// Update indicator
		m_aliveIndicator = false;
		auto & world = m_engine->getModule_World();
		world.removeNotifyOnComponentType("LightPoint_Component", m_notifyLight);
		world.removeNotifyOnComponentType("LightShadow_Component", m_notifyShadow);
	}
	/** Constructor. */
	inline Point_Lighting(Engine * engine, Prop_View * propView)
		: m_engine(engine) {
		// Asset Loading
		m_shader_Lighting = Shared_Shader(m_engine, "Core\\Point\\Light");
		m_shader_Stencil = Shared_Shader(m_engine, "Core\\Point\\Stencil");
		m_shader_Shadow = Shared_Shader(m_engine, "Core\\Point\\Shadow");
		m_shader_Culling = Shared_Shader(m_engine, "Core\\Point\\Culling");
		m_shapeSphere = Shared_Primitive(m_engine, "sphere");

		// Preferences
		auto & preferences = m_engine->getPreferenceState();
		preferences.getOrSetValue(PreferenceState::C_SHADOW_QUALITY, m_updateQuality);
		preferences.addCallback(PreferenceState::C_SHADOW_QUALITY, m_aliveIndicator, [&](const float &f) { m_updateQuality = (unsigned int)f; });
		preferences.getOrSetValue(PreferenceState::C_SHADOW_SIZE_POINT, m_shadowSize.x);
		preferences.addCallback(PreferenceState::C_SHADOW_SIZE_POINT, m_aliveIndicator, [&](const float &f) {
			m_outOfDate = true;
			m_shadowSize = glm::ivec2(std::max(1, (int)f));
			m_shadowFBO.resize(m_shadowSize, (unsigned int)(m_shadowBuffer.getLength()) * 12u);
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
		addComponentType(Transform_Component::ID, FLAG_OPTIONAL);
		GLuint data[] = { 0,0,0,0 };
		m_indirectShape.write(0, sizeof(GLuint) * 4, &data);

		// Error Reporting
		if (!isValid())
			engine->getManager_Messages().error("Invalid ECS System: Point_Lighting");

		// Add New Component Types
		auto & world = m_engine->getModule_World();
		world.addLevelListener(&m_outOfDate);
		m_notifyLight = world.addNotifyOnComponentType("LightPoint_Component", [&](BaseECSComponent * c) {
			auto * component = (LightPoint_Component*)c;
			component->m_lightIndex = m_lightBuffer.newElement();
		});
		m_notifyShadow = world.addNotifyOnComponentType("LightPointShadow_Component", [&](BaseECSComponent * c) {
			auto * component = (LightPointShadow_Component*)c;
			auto shadowSpot = (int)(m_shadowBuffer.getLength() * 12);
			component->m_shadowIndex = m_shadowBuffer.newElement();
			m_shadowBuffer[*component->m_shadowIndex].Shadow_Spot = shadowSpot;
			component->m_shadowSpot = shadowSpot;
			m_shadowFBO.resize(m_shadowFBO.m_size, shadowSpot + 12);
			m_shadowBuffer[*component->m_shadowIndex].lightV = glm::mat4(1.0f);

			// Default Values
			for (int x = 0; x < 6; ++x) {
				m_shadowBuffer[*component->m_shadowIndex].lightPV[x] = glm::mat4(1.0f);
				m_shadowBuffer[*component->m_shadowIndex].inversePV[x] = glm::mat4(1.0f);
			}
		});
	}


	// Public Interface Implementations
	inline virtual void applyTechnique(const float & deltaTime) override {
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
			Transform_Component * transformComponent = (Transform_Component*)componentParam[2];
			const auto & index = *lightComponent->m_lightIndex;

			// Sync Transform Attributes
			if (transformComponent) {
				const auto & position = transformComponent->m_transform.m_position;
				lightComponent->m_position = position;
				const glm::mat4 trans = glm::translate(glm::mat4(1.0f), position);
				const glm::mat4 scl = glm::scale(glm::mat4(1.0f), glm::vec3(lightComponent->m_radius*lightComponent->m_radius)*1.1f);
				m_lightBuffer[index].mMatrix = (trans)* scl;

				if (shadowComponent) {
					const auto & shadowIndex = *shadowComponent->m_shadowIndex;
					m_shadowBuffer[shadowIndex].lightV = glm::translate(glm::mat4(1.0f), -position);
					glm::mat4 rotMats[6];
					const glm::mat4 pMatrix = glm::perspective(glm::radians(90.0f), 1.0f, 0.01f, lightComponent->m_radius*lightComponent->m_radius);
					rotMats[0] = pMatrix * glm::lookAt(position, position + glm::vec3(1, 0, 0), glm::vec3(0, -1, 0));
					rotMats[1] = pMatrix * glm::lookAt(position, position + glm::vec3(-1, 0, 0), glm::vec3(0, -1, 0));
					rotMats[2] = pMatrix * glm::lookAt(position, position + glm::vec3(0, 1, 0), glm::vec3(0, 0, 1));
					rotMats[3] = pMatrix * glm::lookAt(position, position + glm::vec3(0, -1, 0), glm::vec3(0, 0, -1));
					rotMats[4] = pMatrix * glm::lookAt(position, position + glm::vec3(0, 0, 1), glm::vec3(0, -1, 0));
					rotMats[5] = pMatrix * glm::lookAt(position, position + glm::vec3(0, 0, -1), glm::vec3(0, -1, 0));
					for (int x = 0; x < 6; ++x) {
						m_shadowBuffer[shadowIndex].lightPV[x] = rotMats[x];
						m_shadowBuffer[shadowIndex].inversePV[x] = glm::inverse(rotMats[x]);
					}
				}
			}

			// Update Buffer Attributes
			m_lightBuffer[index].LightColor = lightComponent->m_color;
			m_lightBuffer[index].LightPosition = lightComponent->m_position;
			m_lightBuffer[index].LightIntensity = lightComponent->m_intensity;
			m_lightBuffer[index].LightRadius = lightComponent->m_radius;
			lightIndices.push_back((GLuint)index);
			if (shadowComponent) {
				if (m_outOfDate)
					shadowComponent->m_outOfDate = true;
				shadowIndices.push_back((GLint)*shadowComponent->m_shadowIndex);
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
		m_shadowFBO.bindForWriting();
		for (const auto[light, shadow] : m_shadowsToUpdate) {
			// Update static shadows
			if (shadow->m_outOfDate || m_outOfDate) {
				m_shadowFBO.clear(shadow->m_shadowSpot + 6);
				m_propShadow_Static->setData(light->m_position, (int)*light->m_lightIndex, (int)*shadow->m_shadowIndex);
				// Update components
				world.updateSystem(m_propShadow_Static, deltaTime);
				// Render components
				m_propShadow_Static->applyTechnique(deltaTime);
				shadow->m_outOfDate = false;
			}
			// Update dynamic shadows
			m_shadowFBO.clear(shadow->m_shadowSpot);
			m_propShadow_Dynamic->setData(light->m_position, (int)*light->m_lightIndex, (int)*shadow->m_shadowIndex);
			// Update components
			world.updateSystem(m_propShadow_Dynamic, deltaTime);
			// Render components
			m_propShadow_Dynamic->applyTechnique(deltaTime);
			shadow->m_updateTime = m_engine->getTime();
		}

		if (m_outOfDate)
			m_outOfDate = false;
		glViewport(0, 0, GLsizei((*m_cameraBuffer)->Dimensions.x), GLsizei((*m_cameraBuffer)->Dimensions.y));
	}
	/** Render all the lights. */
	inline void renderLights(const float & deltaTime) {
		glEnable(GL_STENCIL_TEST);
		glEnable(GL_BLEND);
		glBlendEquation(GL_FUNC_ADD);
		glBlendFunc(GL_ONE, GL_ONE);

		// Draw only into depth-stencil buffer
		m_shader_Stencil->bind();									// Shader (point)
		m_gfxFBOS->bindForWriting("LIGHTING");						// Ensure writing to lighting FBO
		m_gfxFBOS->bindForReading("GEOMETRY", 0);					// Read from Geometry FBO
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
	Shared_Shader m_shader_Lighting, m_shader_Stencil, m_shader_Shadow, m_shader_Culling;
	Shared_Primitive m_shapeSphere;
	Prop_Shadow * m_propShadow_Static = nullptr, *m_propShadow_Dynamic = nullptr;
	GLuint m_updateQuality = 1u;
	glm::ivec2 m_shadowSize = glm::ivec2(512);
	StaticBuffer m_indirectShape = StaticBuffer(sizeof(GLuint) * 4);
	DynamicBuffer m_visLights, m_visShadows;
	std::vector<std::pair<LightPoint_Component*, LightPointShadow_Component*>> m_shadowsToUpdate;
	bool m_outOfDate = true;
	std::shared_ptr<bool> m_aliveIndicator = std::make_shared<bool>(true);
	int m_notifyLight = -1, m_notifyShadow = -1;

	// Core Lighting Data
	/** OpenGL buffer for point lights. */
	struct Point_Buffer {
		glm::mat4 mMatrix;
		glm::vec3 LightColor; float padding1;
		glm::vec3 LightPosition; float padding2;
		float LightIntensity;
		float LightRadius; glm::vec2 padding3;
	};
	/** OpenGL buffer for point light shadows. */
	struct Point_Shadow_Buffer {
		glm::mat4 lightV;
		glm::mat4 lightPV[6];
		glm::mat4 inversePV[6];
		int Shadow_Spot; glm::vec3 padding1;
	};
	GL_ArrayBuffer<Point_Buffer> m_lightBuffer;
	GL_ArrayBuffer<Point_Shadow_Buffer> m_shadowBuffer;
	FBO_Shadow_Point m_shadowFBO;
};

#endif // DIRECTIONAL_LIGHTING_H