#pragma once
#ifndef SPOT_LIGHTING_H
#define SPOT_LIGHTING_H

#include "Modules/Graphics/Common/Graphics_Technique.h"
#include "Modules/Graphics/Lighting/components.h"
#include "Modules/Graphics/Lighting/FBO_Shadow_Spot.h"
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


/***/
class Spot_Lighting : public Graphics_Technique {
public:
	// Public (de)Constructors
	/** Destructor. */
	inline ~Spot_Lighting() {
		// Update indicator
		m_aliveIndicator = false;
		auto & world = m_engine->getModule_World();
		world.removeNotifyOnComponentType("LightSpot_Component", m_notifyLight);
		world.removeNotifyOnComponentType("LightSpotShadow_Component", m_notifyShadow);
	}
	/** Constructor. */
	inline Spot_Lighting(Engine * engine, Prop_View * propView)
		: m_engine(engine) {
		// Asset Loading
		m_shader_Lighting = Shared_Shader(m_engine, "Core\\Spot\\Light");
		m_shader_Stencil = Shared_Shader(m_engine, "Core\\Spot\\Stencil");
		m_shader_Shadow = Shared_Shader(m_engine, "Core\\Spot\\Shadow");
		m_shader_Culling = Shared_Shader(m_engine, "Core\\Spot\\Culling");
		m_shapeCone = Shared_Primitive(m_engine, "cone");

		// Preferences
		auto & preferences = m_engine->getPreferenceState();
		preferences.getOrSetValue(PreferenceState::C_SHADOW_QUALITY, m_updateQuality);
		preferences.addCallback(PreferenceState::C_SHADOW_QUALITY, m_aliveIndicator, [&](const float &f) { m_updateQuality = (unsigned int)f; });
		preferences.getOrSetValue(PreferenceState::C_SHADOW_SIZE_SPOT, m_shadowSize.x);
		preferences.addCallback(PreferenceState::C_SHADOW_SIZE_SPOT, m_aliveIndicator, [&](const float &f) {
			m_outOfDate = true;
			m_shadowSize = glm::ivec2(std::max(1, (int)f));
			m_shadowFBO.resize(m_shadowSize, m_shadowBuffer.getLength() * 2);
			if (m_shader_Lighting && m_shader_Lighting->existsYet())
				m_shader_Lighting->addCallback(m_aliveIndicator, [&](void) { m_shader_Lighting->setUniform(0, 1.0f / m_shadowSize.x); });
		});
		m_shadowSize = glm::ivec2(std::max(1, m_shadowSize.x));
		m_shadowFBO.resize(m_shadowSize, 1);

		// Asset-Finished Callbacks
		m_shapeCone->addCallback(m_aliveIndicator, [&]() mutable {
			const GLuint data = { (GLuint)m_shapeCone->getSize() };
			m_indirectShape.write(0, sizeof(GLuint), &data); // count, primCount, first, reserved
		});
		m_shader_Lighting->addCallback(m_aliveIndicator, [&](void) { m_shader_Lighting->setUniform(0, 1.0f / (float)m_shadowSize.x); });

		// Geometry rendering pipeline
		m_propShadow_Static = new Prop_Shadow(m_engine, 1, Prop_Shadow::RenderStatic, m_shader_Culling, m_shader_Shadow, propView);
		m_propShadow_Dynamic = new Prop_Shadow(m_engine, 1, Prop_Shadow::RenderDynamic, m_shader_Culling, m_shader_Shadow, propView);
		
		// Error Reporting
		if (glCheckNamedFramebufferStatus(m_shadowFBO.m_fboID, GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			m_engine->getManager_Messages().error("Spot_Lighting Shadowmap Framebuffer has encountered an error.");
		
		// Declare component types used
		addComponentType(LightSpot_Component::ID);
		addComponentType(LightSpotShadow_Component::ID, FLAG_OPTIONAL);
		addComponentType(Transform_Component::ID, FLAG_OPTIONAL);
		GLuint data[] = { 0,0,0,0 };
		m_indirectShape.write(0, sizeof(GLuint) * 4, &data);

		// Error Reporting
		if (!isValid())
			m_engine->getManager_Messages().error("Invalid ECS System: Spot_Lighting");

		// Add New Component Types
		auto & world = m_engine->getModule_World();
		world.addLevelListener(&m_outOfDate);	
		m_notifyLight = world.addNotifyOnComponentType("LightSpot_Component", [&](BaseECSComponent * c) {
			auto * component = (LightSpot_Component*)c;
			component->m_lightIndex = m_lightBuffer.newElement();
		});
		m_notifyShadow = world.addNotifyOnComponentType("LightSpotShadow_Component", [&](BaseECSComponent * c) {
			auto * component = (LightSpotShadow_Component*)c;
			auto shadowSpot = (int)(m_shadowBuffer.getLength() * 2);
			component->m_shadowIndex = m_shadowBuffer.newElement();
			m_shadowBuffer[*component->m_shadowIndex].Shadow_Spot = shadowSpot;
			component->m_shadowSpot = shadowSpot;
			m_shadowFBO.resize(m_shadowFBO.m_size, shadowSpot + 2);

			// Default Values
			m_shadowBuffer[*component->m_shadowIndex].lightV = glm::mat4(1.0f);
			m_shadowBuffer[*component->m_shadowIndex].lightPV = glm::mat4(1.0f);
			m_shadowBuffer[*component->m_shadowIndex].inversePV = glm::mat4(1.0f);
		});
	}


	// Public Interface Implementations
	inline virtual void applyTechnique(const float & deltaTime) override {
		// Exit Early
		if (!m_enabled || !m_shapeCone->existsYet() || !m_shader_Lighting->existsYet() || !m_shader_Stencil->existsYet() || !m_shader_Shadow->existsYet() || !m_shader_Culling->existsYet())
			return;

		// Bind buffers common for rendering and shadowing
		m_lightBuffer.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 8); // Light buffer
		m_shadowBuffer.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 9); // Shadow buffer

		// Render important shadows
		renderShadows(deltaTime);
		// Render lights
		renderLights(deltaTime);
	}
	inline virtual void updateComponents(const float & deltaTime, const std::vector< std::vector<BaseECSComponent*> > & components) override {
		// Accumulate Light Data	
		std::vector<GLint> lightIndices, shadowIndices;
		PriorityList<float, std::pair<LightSpot_Component*, LightSpotShadow_Component*>, std::less<float>> oldest;
		for each (const auto & componentParam in components) {
			LightSpot_Component * lightComponent = (LightSpot_Component*)componentParam[0];
			LightSpotShadow_Component * shadowComponent = (LightSpotShadow_Component*)componentParam[1];
			Transform_Component * transformComponent = (Transform_Component*)componentParam[2];
			const auto & index = *lightComponent->m_lightIndex;

			// Sync Transform Attributes
			if (transformComponent) {
				const auto & position = transformComponent->m_transform.m_position;
				const auto & orientation = transformComponent->m_transform.m_orientation;
				const auto matRot = glm::mat4_cast(orientation);
				lightComponent->m_position = position;
				auto dir = matRot * glm::vec4(0, 0, -1, 1);
				dir /= dir.w;
				m_lightBuffer[index].LightDirection = glm::normalize(glm::vec3(dir));
				const glm::mat4 trans = glm::translate(glm::mat4(1.0f), position);
				const glm::mat4 scl = glm::scale(glm::mat4(1.0f), glm::vec3(lightComponent->m_radius * lightComponent->m_radius)*1.1f);
				m_lightBuffer[index].mMatrix = trans * matRot * scl;

				if (shadowComponent) {
					const auto & shadowIndex = *shadowComponent->m_shadowIndex;
					const glm::mat4 pMatrix = glm::perspective(glm::radians(lightComponent->m_cutoff * 2.0f), 1.0f, 0.01f, lightComponent->m_radius * lightComponent->m_radius);
					const glm::mat4 trans = glm::translate(glm::mat4(1.0f), position);
					m_shadowBuffer[shadowIndex].lightV = trans;
					const auto pv = pMatrix * glm::inverse(trans * glm::mat4_cast(orientation));
					m_shadowBuffer[shadowIndex].lightPV = pv;
					m_shadowBuffer[shadowIndex].inversePV = glm::inverse(pv);
				}
			}
			// Update Buffer Attributes
			m_lightBuffer[index].LightColor = lightComponent->m_color;
			m_lightBuffer[index].LightPosition = lightComponent->m_position;
			m_lightBuffer[index].LightIntensity = lightComponent->m_intensity;
			m_lightBuffer[index].LightRadius = lightComponent->m_radius;
			m_lightBuffer[index].LightCutoff = cosf(glm::radians(lightComponent->m_cutoff));
			lightIndices.push_back(index);
			if (shadowComponent) {
				if (m_outOfDate)
					shadowComponent->m_outOfDate = true;
				shadowIndices.push_back(*shadowComponent->m_shadowIndex);
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
				m_shadowFBO.clear(shadow->m_shadowSpot + 1);
				m_propShadow_Static->setData(light->m_position, *light->m_lightIndex, *shadow->m_shadowIndex);
				// Update components
				world.updateSystem(m_propShadow_Static, deltaTime);
				// Render components
				m_propShadow_Static->applyTechnique(deltaTime);
				shadow->m_outOfDate = false;
			}
			// Update dynamic shadows
			m_shadowFBO.clear(shadow->m_shadowSpot);
			m_propShadow_Dynamic->setData(light->m_position, *light->m_lightIndex, *shadow->m_shadowIndex);
			// Update components
			world.updateSystem(m_propShadow_Dynamic, deltaTime);
			// Render components
			m_propShadow_Dynamic->applyTechnique(deltaTime);
			shadow->m_updateTime = m_engine->getTime();
		}

		if (m_outOfDate)
			m_outOfDate = false;
		glViewport(0, 0, (*m_cameraBuffer)->Dimensions.x, (*m_cameraBuffer)->Dimensions.y);
	}
	/** Render all the lights. */
	inline void renderLights(const float & deltaTime) {
		glEnable(GL_STENCIL_TEST);
		glEnable(GL_BLEND);
		glBlendEquation(GL_FUNC_ADD);
		glBlendFunc(GL_ONE, GL_ONE);

		// Draw only into depth-stencil buffer
		m_shader_Stencil->bind();									// Shader (spot)
		m_gfxFBOS->bindForWriting("LIGHTING");							// Ensure writing to lighting FBO
		m_gfxFBOS->bindForReading("GEOMETRY", 0);							// Read from Geometry FBO
		glBindTextureUnit(4, m_shadowFBO.m_textureIDS[2]);			// Shadow map (depth texture)
		m_visLights.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 3);	// SSBO visible light indices
		m_visShadows.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 4);	// SSBO visible shadow indices
		m_indirectShape.bindBuffer(GL_DRAW_INDIRECT_BUFFER);		// Draw call buffer
		glBindVertexArray(m_shapeCone->m_vaoID);					// Quad VAO
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
	inline std::vector<std::pair<LightSpot_Component*, LightSpotShadow_Component*>> PQtoVector(PriorityList<float, std::pair<LightSpot_Component*, LightSpotShadow_Component*>, std::less<float>> oldest) const {
		PriorityList<float, std::pair<LightSpot_Component*, LightSpotShadow_Component*>, std::greater<float>> m_closest(m_updateQuality / 2);
		std::vector<std::pair<LightSpot_Component*, LightSpotShadow_Component*>> outList;
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
	Shared_Primitive m_shapeCone;
	Prop_Shadow * m_propShadow_Static = nullptr, * m_propShadow_Dynamic = nullptr;
	GLuint m_updateQuality = 1u;
	glm::ivec2 m_shadowSize = glm::ivec2(256);
	StaticBuffer m_indirectShape = StaticBuffer(sizeof(GLuint) * 4);
	DynamicBuffer m_visLights, m_visShadows;
	std::vector<std::pair<LightSpot_Component*, LightSpotShadow_Component*>> m_shadowsToUpdate;
	bool m_outOfDate = true;
	std::shared_ptr<bool> m_aliveIndicator = std::make_shared<bool>(true);
	int m_notifyLight = -1, m_notifyShadow = -1;

	// Core Lighting Data
	/** OpenGL buffer for spot lights. */
	struct Spot_Buffer {
		glm::mat4 mMatrix;
		glm::vec3 LightColor; float padding1;
		glm::vec3 LightPosition; float padding2;
		glm::vec3 LightDirection; float padding3;
		float LightIntensity;
		float LightRadius;
		float LightCutoff; float padding4;
	};
	/** OpenGL buffer for spot light shadows. */
	struct Spot_Shadow_Buffer {
		glm::mat4 lightV;
		glm::mat4 lightPV;
		glm::mat4 inversePV;
		int Shadow_Spot; glm::vec3 padding1;
	};
	GL_ArrayBuffer<Spot_Buffer> m_lightBuffer;
	GL_ArrayBuffer<Spot_Shadow_Buffer> m_shadowBuffer;
	FBO_Shadow_Spot m_shadowFBO;
};

#endif // SPOT_LIGHTING_H