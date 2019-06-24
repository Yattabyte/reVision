#pragma once
#ifndef POINT_LIGHTING_H
#define POINT_LIGHTING_H

#include "Modules/Graphics/Common/Graphics_Technique.h"
#include "Modules/Graphics/Lighting/components.h"
#include "Modules/Graphics/Lighting/FBO_Shadow_Point.h"
#include "Modules/Graphics/Geometry/Prop_Shadow.h"
#include "Modules/World/ECS/components.h"
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
	}
	/** Constructor. */
	inline Point_Lighting(Engine * engine, Prop_View * propView)
		: m_engine(engine), Graphics_Technique(PRIMARY_LIGHTING) {
		addComponentType(Renderable_Component::ID, FLAG_REQUIRED);
		addComponentType(LightPoint_Component::ID, FLAG_REQUIRED);
		addComponentType(LightColor_Component::ID, FLAG_OPTIONAL);
		addComponentType(LightRadius_Component::ID, FLAG_OPTIONAL);
		addComponentType(Transform_Component::ID, FLAG_OPTIONAL);
		addComponentType(BoundingSphere_Component::ID, FLAG_OPTIONAL);

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
			m_shadowFBO.resize(m_shadowSize, (m_shadowCount + m_unusedShadows.size()) * 12);
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

		GLuint data[] = { 0,0,0,0 };
		m_indirectShape.write(0, sizeof(GLuint) * 4, &data);

		// Add New Component Types
		auto & world = m_engine->getModule_World();
		world.addNotifyOnComponentType(LightPoint_Component::ID, m_aliveIndicator, [&](BaseECSComponent * c) {
			((LightPoint_Component*)c)->m_lightIndex = m_lightBuffer.newElement();
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
		PriorityList<float, LightPoint_Component*, std::less<float>> oldest;
		std::vector<GLint> lightIndices, shadowIndices;
		for each (const auto & componentParam in components) {
			Renderable_Component * renderableComponent = (Renderable_Component*)componentParam[0];
			LightPoint_Component * lightComponent = (LightPoint_Component*)componentParam[1];
			LightColor_Component * colorComponent = (LightColor_Component*)componentParam[2];
			LightRadius_Component * radiusComponent = (LightRadius_Component*)componentParam[3];
			Transform_Component * transformComponent = (Transform_Component*)componentParam[4];
			BoundingSphere_Component * bsphereComponent = (BoundingSphere_Component*)componentParam[5];
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

				// Sync Transform Attributes
				if (transformComponent) {
					const auto & position = transformComponent->m_transform.m_position;
					lightComponent->m_position = position;
					const glm::mat4 trans = glm::translate(glm::mat4(1.0f), position);
					const glm::mat4 scl = glm::scale(glm::mat4(1.0f), glm::vec3(radiusSquared *1.1f));
					m_lightBuffer[index].mMatrix = (trans)* scl;

					if (lightComponent->m_hasShadow) {
						m_lightBuffer[index].lightV = glm::translate(glm::mat4(1.0f), -position);
						glm::mat4 rotMats[6];
						const glm::mat4 pMatrix = glm::perspective(glm::radians(90.0f), 1.0f, 0.01f, radiusSquared);
						rotMats[0] = pMatrix * glm::lookAt(position, position + glm::vec3(1, 0, 0), glm::vec3(0, -1, 0));
						rotMats[1] = pMatrix * glm::lookAt(position, position + glm::vec3(-1, 0, 0), glm::vec3(0, -1, 0));
						rotMats[2] = pMatrix * glm::lookAt(position, position + glm::vec3(0, 1, 0), glm::vec3(0, 0, 1));
						rotMats[3] = pMatrix * glm::lookAt(position, position + glm::vec3(0, -1, 0), glm::vec3(0, 0, -1));
						rotMats[4] = pMatrix * glm::lookAt(position, position + glm::vec3(0, 0, 1), glm::vec3(0, -1, 0));
						rotMats[5] = pMatrix * glm::lookAt(position, position + glm::vec3(0, 0, -1), glm::vec3(0, -1, 0));
						for (int x = 0; x < 6; ++x) {
							m_lightBuffer[index].lightPV[x] = rotMats[x];
							m_lightBuffer[index].inversePV[x] = glm::inverse(rotMats[x]);
						}
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
						shadowSpot = (int)(m_shadowCount) * 12;
					lightComponent->m_shadowSpot = shadowSpot;
					m_shadowCount++;
					m_shadowFBO.resize(m_shadowSize, (m_shadowCount + m_unusedShadows.size()) * 12);
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
		if (!m_enabled || !m_shapeSphere->existsYet() || !m_shader_Lighting->existsYet() || !m_shader_Stencil->existsYet())
			return;

		// Render direct lights
		renderLights(deltaTime, viewport);
	}


private:
	// Private Methods	
	/** Render all the geometry from each light.
	@param	deltaTime	the amount of time passed since last frame.*/
	inline void updateShadows(const float & deltaTime) {
		m_lightBuffer.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 8);
		glViewport(0, 0, m_shadowSize.x, m_shadowSize.y);
		m_shadowFBO.bindForWriting();
		for (auto * light : m_shadowsToUpdate) {
			// Update static shadows
			if (light->m_outOfDate || m_outOfDate) {
				m_shadowFBO.clear(light->m_shadowSpot + 6);
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
			m_shader_Stencil->bind();									// Shader (point)
			viewport->m_gfxFBOS->bindForWriting("LIGHTING");			// Ensure writing to lighting FBO
			viewport->m_gfxFBOS->bindForReading("GEOMETRY", 0);			// Read from Geometry FBO
			glBindTextureUnit(4, m_shadowFBO.m_textureIDS[0]);			// Shadow map(linear depth texture)
			m_visLights.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 3);	// SSBO visible light indices
			m_lightBuffer.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 8);
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
	}
	/** Converts a priority queue into an stl vector.*/
	inline std::vector<LightPoint_Component*> PQtoVector(PriorityList<float, LightPoint_Component*, std::less<float>> oldest) const {
		PriorityList<float, LightPoint_Component*, std::greater<float>> m_closest(m_updateQuality / 2);
		std::vector<LightPoint_Component*> outList;
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
	Shared_Primitive m_shapeSphere;
	Prop_Shadow * m_propShadow_Static = nullptr, *m_propShadow_Dynamic = nullptr;
	GLuint m_updateQuality = 1u;
	glm::ivec2 m_shadowSize = glm::ivec2(512);
	StaticBuffer m_indirectShape = StaticBuffer(sizeof(GLuint) * 4);
	std::vector<LightPoint_Component*> m_shadowsToUpdate;
	bool m_outOfDate = false;
	std::shared_ptr<bool> m_aliveIndicator = std::make_shared<bool>(true);

	// Visibility
	size_t m_visLightCount = 0ull;
	DynamicBuffer m_visLights;

	// Core Lighting Data
	/** OpenGL buffer for point lights. */
	struct Point_Buffer {
		glm::mat4 lightV;
		glm::mat4 lightPV[6];
		glm::mat4 inversePV[6];
		glm::mat4 mMatrix;
		glm::vec3 LightColor; float padding1;
		glm::vec3 LightPosition; float padding2;
		float LightIntensity;
		float LightRadius;
		int Shadow_Spot; float padding3;
	};
	size_t m_shadowCount = 0ull;
	std::vector<int> m_unusedShadows;
	GL_ArrayBuffer<Point_Buffer> m_lightBuffer;
	FBO_Shadow_Point m_shadowFBO;
};

#endif // DIRECTIONAL_LIGHTING_H