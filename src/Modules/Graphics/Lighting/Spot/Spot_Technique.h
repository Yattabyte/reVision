#pragma once
#ifndef SPOT_TECHNIQUE_H
#define SPOT_TECHNIQUE_H

#include "Modules/Graphics/Common/Graphics_Technique.h"
#include "Modules/Graphics/Lighting/components.h"
#include "Modules/Graphics/Lighting/Spot/FBO_Shadow_Spot.h"
#include "Modules/Graphics/Geometry/Prop/Prop_Shadow.h"
#include "Modules/World/ECS/components.h"
#include "Assets/Shader.h"
#include "Assets/Primitive.h"
#include "Utilities/GL/StaticBuffer.h"
#include "Utilities/GL/DynamicBuffer.h"
#include "Utilities/GL/GL_ArrayBuffer.h"
#include "Utilities/PriorityList.h"
#include "Engine.h"
#include <vector>


/** A core lighting technique responsible for all spot lights. */
class Spot_Technique : public Graphics_Technique {
public:
	// Public (de)Constructors
	/** Destructor. */
	inline ~Spot_Technique() {
		// Update indicator
		m_aliveIndicator = false;
	}
	/** Constructor. */
	inline Spot_Technique(Engine * engine, Prop_Technique * propView)
		: m_engine(engine), Graphics_Technique(PRIMARY_LIGHTING) {
		addComponentType(Renderable_Component::ID, FLAG_REQUIRED);
		addComponentType(LightSpot_Component::ID, FLAG_REQUIRED);
		addComponentType(LightColor_Component::ID, FLAG_OPTIONAL);
		addComponentType(LightRadius_Component::ID, FLAG_OPTIONAL);
		addComponentType(LightCutoff_Component::ID, FLAG_OPTIONAL);
		addComponentType(Transform_Component::ID, FLAG_OPTIONAL);
		addComponentType(BoundingSphere_Component::ID, FLAG_OPTIONAL);

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
			m_shadowFBO.resize(m_shadowSize, (m_shadowCount + m_unusedShadows.size()) * 2);
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
			m_engine->getManager_Messages().error("Spot_Technique Shadowmap Framebuffer has encountered an error.");

		GLuint data[] = { 0,0,0,0 };
		m_indirectShape.write(0, sizeof(GLuint) * 4, &data);

		// Add New Component Types
		auto & world = m_engine->getModule_World();
		world.addNotifyOnComponentType(LightSpot_Component::ID, m_aliveIndicator, [&](BaseECSComponent * c) {
			((LightSpot_Component*)c)->m_lightIndex = m_lightBuffer.newElement();
		});

		// World-Changed Callback
		world.addLevelListener(m_aliveIndicator, [&](const World_Module::WorldState & state) {
			if (state == World_Module::unloaded) {
				clear();
				m_outOfDate = false;
			}
			else if (state == World_Module::finishLoading || state == World_Module::updated)
				m_outOfDate = true;
		});
	}


	// Public Interface Implementations
	inline virtual void beginFrame(const float & deltaTime) override {
		m_lightBuffer.beginWriting();
		m_visLights.beginWriting();
	}
	inline virtual void endFrame(const float & deltaTime) override {
		m_lightBuffer.endWriting();
		m_visLights.endWriting();
	}
	inline virtual void updateComponents(const float & deltaTime, const std::vector<std::vector<BaseECSComponent*>> & components) override {
		PriorityList<float, LightSpot_Component*, std::less<float>> oldest;
		std::vector<GLint> lightIndices;
		for each (const auto & componentParam in components) {
			Renderable_Component * renderableComponent = (Renderable_Component*)componentParam[0];
			LightSpot_Component * lightComponent = (LightSpot_Component*)componentParam[1];
			LightColor_Component * colorComponent = (LightColor_Component*)componentParam[2];
			LightRadius_Component * radiusComponent = (LightRadius_Component*)componentParam[3];
			LightCutoff_Component * cutoffComponent = (LightCutoff_Component*)componentParam[4];
			Transform_Component * transformComponent = (Transform_Component*)componentParam[5];
			BoundingSphere_Component * bsphereComponent = (BoundingSphere_Component*)componentParam[6];
			const auto & index = lightComponent->m_lightIndex;

			// Synchronize the component if it is visible
			if (renderableComponent->m_visible) {
				lightIndices.push_back((GLuint)* lightComponent->m_lightIndex);

				// Sync Color Attributes
				if (colorComponent) {
					m_lightBuffer[index].LightColor = colorComponent->m_color;
					m_lightBuffer[index].LightIntensity = colorComponent->m_intensity;
				}

				// Sync Radius Attributes
				float radiusSquared = 1.0f;
				if (radiusComponent) {
					m_lightBuffer[index].LightRadius = radiusComponent->m_radius;
					radiusSquared = radiusComponent->m_radius * radiusComponent->m_radius;
				}
				if (bsphereComponent)
					bsphereComponent->m_radius = radiusSquared;

				// Sync Cutoff Attributes
				float cutoff = 90.0f;
				if (cutoffComponent) {
					m_lightBuffer[index].LightCutoff = cosf(glm::radians(cutoffComponent->m_cutoff));
					cutoff = cutoffComponent->m_cutoff;
				}

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
					const glm::mat4 scl = glm::scale(glm::mat4(1.0f), glm::vec3(radiusSquared)*1.1f);
					m_lightBuffer[index].mMatrix = trans * matRot * scl;

					if (lightComponent->m_hasShadow) {
						const glm::mat4 pMatrix = glm::perspective(glm::radians(cutoff * 2.0f), 1.0f, 0.01f, radiusSquared);
						const glm::mat4 trans = glm::translate(glm::mat4(1.0f), position);
						m_lightBuffer[index].lightV = trans;
						const auto pv = pMatrix * glm::inverse(trans * glm::mat4_cast(orientation));
						m_lightBuffer[index].lightPV = pv;
						m_lightBuffer[index].inversePV = glm::inverse(pv);
					}
				}

				// Shadowmap logic
				if (lightComponent->m_hasShadow && lightComponent->m_shadowSpot == -1) {
					// Assign shadowmap spot
					int shadowSpot;
					if (m_unusedShadows.size()) {
						// Reclaim a disused shadow map index
						shadowSpot = m_unusedShadows.back();
						m_unusedShadows.pop_back();
					}
					else
						shadowSpot = (int)(m_shadowCount) * 2;
					lightComponent->m_shadowSpot = shadowSpot;
					m_shadowCount++;
					m_shadowFBO.resize(m_shadowSize, (m_shadowCount + m_unusedShadows.size()) * 2);
				}
				else if (!lightComponent->m_hasShadow && lightComponent->m_shadowSpot >= 0) {
					// Save the old shadow index
					m_unusedShadows.push_back(lightComponent->m_shadowSpot);
					lightComponent->m_shadowSpot = -1;
					m_shadowCount--;
				}

				// Sync Buffer Attributes
				m_lightBuffer[index].LightPosition = lightComponent->m_position;
				m_lightBuffer[index].Shadow_Spot = lightComponent->m_shadowSpot;
			}

			// Find oldest shadows always (may see shadow but not light source)
			if (lightComponent->m_hasShadow)
				oldest.insert(lightComponent->m_updateTime, lightComponent);
		}

		// Update Draw Buffers
		m_visLightCount = lightIndices.size();
		m_visLights.write(0, sizeof(GLuint) * m_visLightCount, lightIndices.data());
		m_indirectShape.write(sizeof(GLuint), sizeof(GLuint), &m_visLightCount); // update primCount (2nd param)

		// Shadows are updated in rendering pass, determined once a frame
		m_shadowsToUpdate = PQtoVector(oldest);
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
		if (!m_enabled || !m_shapeCone->existsYet() || !m_shader_Lighting->existsYet() || !m_shader_Stencil->existsYet() || !m_shader_Shadow->existsYet() || !m_shader_Culling->existsYet())
			return;

		// Render lights
		renderLights(deltaTime, viewport);
	}


private:
	// Private Methods
	/** Render all the geometry from each light.
	@param	deltaTime	the amount of time passed since last frame. */
	inline void updateShadows(const float & deltaTime) {
		if (m_shadowsToUpdate.size()) {
			glViewport(0, 0, m_shadowSize.x, m_shadowSize.y);
			m_lightBuffer.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 8);
			m_shadowFBO.bindForWriting();
			for (auto * light : m_shadowsToUpdate) {
				// Update static shadows
				if (light->m_outOfDate || m_outOfDate) {
					m_shadowFBO.clear(light->m_shadowSpot + 1);
					// Render components
					m_propShadow_Static->setData(light->m_position, (int)*light->m_lightIndex);
					m_propShadow_Static->renderShadows(deltaTime);
					light->m_outOfDate = false;
				}
				// Update dynamic shadows
				m_shadowFBO.clear(light->m_shadowSpot);
				// Render components
				m_propShadow_Dynamic->setData(light->m_position, (int)*light->m_lightIndex);
				m_propShadow_Dynamic->renderShadows(deltaTime);
				light->m_updateTime = m_engine->getTime();
			}
			m_shadowsToUpdate.clear();
			m_outOfDate = false;
		}
	}
	/** Render all the lights.
	@param	deltaTime	the amount of time passed since last frame.
	@param	viewport	the viewport to render from. */
	inline void renderLights(const float & deltaTime, const std::shared_ptr<Viewport> & viewport) {
		if (m_visLightCount) {
			glEnable(GL_STENCIL_TEST);
			glEnable(GL_BLEND);
			glBlendEquation(GL_FUNC_ADD);
			glBlendFunc(GL_ONE, GL_ONE);

			// Draw only into depth-stencil buffer
			m_shader_Stencil->bind();									// Shader (spot)
			viewport->m_gfxFBOS->bindForWriting("LIGHTING");			// Ensure writing to lighting FBO
			viewport->m_gfxFBOS->bindForReading("GEOMETRY", 0);			// Read from Geometry FBO
			glBindTextureUnit(4, m_shadowFBO.m_textureIDS[2]);			// Shadow map (depth texture)
			m_visLights.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 3);	// SSBO visible light indices
			m_lightBuffer.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 8);
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
	}
	/** Converts a priority queue into an stl vector.*/
	inline std::vector<LightSpot_Component*> PQtoVector(PriorityList<float, LightSpot_Component*, std::less<float>> oldest) const {
		PriorityList<float, LightSpot_Component*, std::greater<float>> m_closest(m_updateQuality / 2);
		std::vector<LightSpot_Component*> outList;
		outList.reserve(m_updateQuality);

		for each (const auto &element in oldest.toList()) {
			if (outList.size() < (m_updateQuality / 2))
				outList.push_back(element);
			else
				m_closest.insert(element->m_updateTime, element);
		}

		for each (const auto &element in m_closest.toList()) {
			if (outList.size() >= m_updateQuality)
				break;
			outList.push_back(element);
		}

		return outList;
	}
	/** Clear out the lights and shadows queued up for rendering. */
	inline void clear() {
		const size_t lightSize = 0;
		m_indirectShape.write(sizeof(GLuint), sizeof(GLuint), &lightSize); // update primCount (2nd param)
		m_shadowsToUpdate.clear();
		m_lightBuffer.clear();
	}


	// Private Attributes
	Engine * m_engine = nullptr;
	Shared_Shader m_shader_Lighting, m_shader_Stencil, m_shader_Shadow, m_shader_Culling;
	Shared_Primitive m_shapeCone;
	Prop_Shadow * m_propShadow_Static = nullptr, *m_propShadow_Dynamic = nullptr;
	GLuint m_updateQuality = 1u;
	glm::ivec2 m_shadowSize = glm::ivec2(256);
	StaticBuffer m_indirectShape = StaticBuffer(sizeof(GLuint) * 4);
	std::vector<LightSpot_Component*> m_shadowsToUpdate;
	bool m_outOfDate = false;
	std::shared_ptr<bool> m_aliveIndicator = std::make_shared<bool>(true);

	// Visibility
	size_t m_visLightCount = 0ull;
	DynamicBuffer m_visLights;

	// Core Lighting Data
	/** OpenGL buffer for spot lights. */
	struct Spot_Buffer {
		glm::mat4 lightV;
		glm::mat4 lightPV;
		glm::mat4 inversePV;
		glm::mat4 mMatrix;
		glm::vec3 LightColor; float padding1;
		glm::vec3 LightPosition; float padding2;
		glm::vec3 LightDirection; float padding3;
		float LightIntensity;
		float LightRadius;
		float LightCutoff;
		int Shadow_Spot;
	};
	size_t m_shadowCount = 0ull;
	std::vector<int> m_unusedShadows;
	GL_ArrayBuffer<Spot_Buffer> m_lightBuffer;
	FBO_Shadow_Spot m_shadowFBO;
};

#endif // SPOT_TECHNIQUE_H