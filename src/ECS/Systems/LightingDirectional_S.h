#pragma once
#ifndef LIGHTINGDIRECTIONAL_S_H
#define LIGHTINGDIRECTIONAL_S_H 

#include "ECS\Systems\ecsSystem.h"
#include "Assets\Asset_Shader.h"
#include "Assets\Asset_Primitive.h"
#include "ECS\Components\LightDirectional_C.h"
#include "ECS\Resources\FBO_Shadow_Directional.h"
#include "ECS\Systems\PropShadowing.h"
#include "Utilities\GL\StaticBuffer.h"
#include "Utilities\GL\DynamicBuffer.h"
#include "Utilities\GL\FBO.h"
#include "Utilities\PriorityList.h"
#include "Engine.h"
#include "GLFW\glfw3.h"
#include <vector>


/** A system that performs lighting operations for directional lights. */
class LightingDirectional_System : public BaseECSSystem {
public: 
	// (de)Constructors
	~LightingDirectional_System() {
		m_engine->removePrefCallback(PreferenceState::C_WINDOW_WIDTH, this);
		m_engine->removePrefCallback(PreferenceState::C_WINDOW_HEIGHT, this);
		m_engine->removePrefCallback(PreferenceState::C_SHADOW_QUALITY, this);
		m_engine->removePrefCallback(PreferenceState::C_SHADOW_SIZE_DIRECTIONAL, this);
		if (m_shapeQuad.get()) m_shapeQuad->removeCallback(this);
		if (m_shader_Lighting.get()) m_shader_Lighting->removeCallback(this);
		glDeleteVertexArrays(1, &m_quadVAO);
	}
	LightingDirectional_System(
		Engine * engine, 
		FBO_Base * geometryFBO, FBO_Base * lightingFBO,
		GL_Vector * propBuffer, GL_Vector * skeletonBuffer
	) : BaseECSSystem() {
		// Declare component types used
		addComponentType(LightDirectional_Component::ID);
		addComponentType(LightDirectionalShadow_Component::ID, FLAG_OPTIONAL);

		// Shared Parameters
		m_engine = engine;
		m_geometryFBO = geometryFBO;
		m_lightingFBO = lightingFBO;
		m_shadowFBO;

		// Asset Loading
		m_shader_Lighting = Asset_Shader::Create(m_engine, "Core\\Directional\\Light");
		m_shader_Shadow = Asset_Shader::Create(m_engine, "Core\\Directional\\Shadow");
		m_shader_Culling = Asset_Shader::Create(m_engine, "Core\\Directional\\Culling");
		m_shapeQuad = Asset_Primitive::Create(engine, "quad");

		// Preference Callbacks
		m_renderSize.x = m_engine->addPrefCallback(PreferenceState::C_WINDOW_WIDTH, this, [&](const float &f) {
			m_renderSize = glm::ivec2(f, m_renderSize.y);
		});
		m_renderSize.y = m_engine->addPrefCallback(PreferenceState::C_WINDOW_HEIGHT, this, [&](const float &f) {
			m_renderSize = glm::ivec2(m_renderSize.x, f);
		});	
		m_updateQuality = m_engine->addPrefCallback(PreferenceState::C_SHADOW_QUALITY, this, [&](const float &f) {m_updateQuality = f; });
		m_shadowSize.x = m_engine->addPrefCallback(PreferenceState::C_SHADOW_SIZE_DIRECTIONAL, this, [&](const float &f) { m_shadowSize = glm::vec2(max(1.0f, f)); });
		m_shadowSize = glm::vec2(max(1.0f, m_shadowSize.x));
		m_shadowFBO.resize(m_shadowSize, 4);
		m_shader_Lighting->addCallback(this, [&](void) {m_shader_Lighting->Set_Uniform(0, 1.0f / m_shadowSize.x); });

		// Primitive Construction
		m_quadVAOLoaded = false;
		m_quadVAO = Asset_Primitive::Generate_VAO();
		m_indirectShape = StaticBuffer(sizeof(GLuint) * 4, 0);
		m_shapeQuad->addCallback(this, [&]() mutable {
			m_quadVAOLoaded = true;
			m_shapeQuad->updateVAO(m_quadVAO);
			const GLuint data = m_shapeQuad->getSize();
			m_indirectShape.write(0, sizeof(GLuint), &data); // count, primCount, first, reserved
		});

		// Geometry rendering pipeline
		m_geometrySystems.addSystem(new PropShadowing_System(engine, 4, PropShadowing_System::RenderAll, m_shader_Culling, m_shader_Shadow, propBuffer, skeletonBuffer));
	}


	// Interface Implementation	
	virtual void updateComponents(const float & deltaTime, const std::vector< std::vector<BaseECSComponent*> > & components) {
		// Exit Early
		if (!m_quadVAOLoaded || !m_shader_Lighting || !m_shader_Lighting->existsYet() || !m_shader_Shadow || !m_shader_Shadow->existsYet() || !m_shader_Culling || !m_shader_Culling->existsYet())
			return;

		// Clear Data
		lightIndices.clear();
		shadowIndices.clear();
		m_oldest.clear();

		// Accumulate Light Data		
		auto & graphics = m_engine->getGraphicsModule();
		const auto cameraBuffer = graphics.m_cameraBuffer.getElement(graphics.getActiveCamera());
		cameraBuffer->wait();
		for each (const auto & componentParam in components) {
			LightDirectional_Component * lightComponent = (LightDirectional_Component*)componentParam[0];
			LightDirectionalShadow_Component * shadowComponent = (LightDirectionalShadow_Component*)componentParam[1];
			lightIndices.push_back(lightComponent->m_data->index);
			if (shadowComponent) {
				shadowComponent->m_data->wait();
				shadowIndices.push_back(shadowComponent->m_data->index);
				m_oldest.insert(shadowComponent->m_updateTime, std::make_pair(lightComponent, shadowComponent));

				const glm::mat4 CamInv = glm::inverse(cameraBuffer->data->vMatrix);

				const glm::vec2 &size = cameraBuffer->data->Dimensions;
				float ar = size.x / size.y;
				float tanHalfHFOV = glm::radians(cameraBuffer->data->FOV) / 2.0f;
				float tanHalfVFOV = atanf(tanf(tanHalfHFOV) / ar);

				for (int i = 0; i < NUM_CASCADES; i++) {
					float points[4] = { 
						shadowComponent->m_cascadeEnd[i] * tanHalfHFOV,
						shadowComponent->m_cascadeEnd[i + 1] * tanHalfHFOV,
						shadowComponent->m_cascadeEnd[i] * tanHalfVFOV,
						shadowComponent->m_cascadeEnd[i + 1] * tanHalfVFOV
					};

					glm::vec3 frustumCorners[8] = {
						// near face65
						glm::vec3(points[0], points[2], shadowComponent->m_cascadeEnd[i]),
						glm::vec3(-points[0], points[2], shadowComponent->m_cascadeEnd[i]),
						glm::vec3(points[0], -points[2], shadowComponent->m_cascadeEnd[i]),
						glm::vec3(-points[0], -points[2], shadowComponent->m_cascadeEnd[i]),
						// far face
						glm::vec3(points[1], points[3], shadowComponent->m_cascadeEnd[i + 1]),
						glm::vec3(-points[1], points[3], shadowComponent->m_cascadeEnd[i + 1]),
						glm::vec3(points[1], -points[3], shadowComponent->m_cascadeEnd[i + 1]),
						glm::vec3(-points[1], -points[3], shadowComponent->m_cascadeEnd[i + 1])
					};

					// Find the middle of current view frustum chunk
					glm::vec3 middle(0, 0, ((shadowComponent->m_cascadeEnd[i + 1] - shadowComponent->m_cascadeEnd[i]) / 2.0f) + shadowComponent->m_cascadeEnd[i]);

					// Measure distance from middle to the furthest point of frustum slice
					// Use to make a bounding sphere, but then convert into a bounding box
					float radius = glm::length(frustumCorners[7] - middle);
					glm::vec3 aabb(radius);

					const glm::vec3 volumeUnitSize = (aabb - -aabb) / m_shadowSize.x;
					const glm::vec3 frustumpos = glm::vec3(shadowComponent->m_mMatrix * CamInv * glm::vec4(middle, 1.0f));
					const glm::vec3 clampedPos = glm::floor((frustumpos + (volumeUnitSize / 2.0f)) / volumeUnitSize) * volumeUnitSize;
					const glm::vec3 newMin = -aabb + clampedPos;
					const glm::vec3 newMax = aabb + clampedPos;

					const float l = newMin.x, r = newMax.x, b = newMax.y, t = newMin.y, n = -newMin.z, f = -newMax.z;
					const glm::mat4 pMatrix = glm::ortho(l, r, b, t, n, f);
					const glm::mat4 pvMatrix = pMatrix * shadowComponent->m_mMatrix;					
					shadowComponent->m_data->data->lightVP[i] = pvMatrix;
					shadowComponent->m_data->data->inverseVP[i] = inverse(pvMatrix);
				}

				for (int x = 0; x < NUM_CASCADES; ++x) {
					const glm::vec4 v1 = glm::vec4(0, 0, shadowComponent->m_cascadeEnd[x + 1], 1.0f);
					const glm::vec4 v2 = cameraBuffer->data->pMatrix * v1;
					shadowComponent->m_data->data->CascadeEndClipSpace[x] = v2.z;
				}
				shadowComponent->m_data->lock();
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
		m_lightBuffer.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 8); // Light buffer
		m_shadowBuffer.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 9); // Shadow buffer

		// Render important shadows
		renderShadows(deltaTime);
		// Render lights
		renderLights(deltaTime);
	}


	// Public Methods
	/** Registers a light component.
	@param	component	the light component to register. */
	void registerComponent(LightDirectional_Component & component) {
		component.m_data = m_lightBuffer.newElement();
		// Default Values
		component.m_data->data->LightColor = glm::vec3(1.0f);
		component.m_data->data->LightDirection = glm::vec3(0, -1.0f, 0);
		component.m_data->data->LightIntensity = 1.0f;
	}
	/** Registers a shadow component.
	@param	component	the shadow component to register. */
	void registerComponent(LightDirectionalShadow_Component & component) {
		float near_plane = -0.1f;
		float far_plane = -m_engine->getPreference(PreferenceState::C_DRAW_DISTANCE);
		component.m_cascadeEnd[0] = near_plane;
		for (int x = 1; x < NUM_CASCADES + 1; ++x) {
			const float cLog = near_plane * powf((far_plane / near_plane), (float(x) / float(NUM_CASCADES)));
			const float cUni = near_plane + ((far_plane - near_plane) * x / NUM_CASCADES);
			const float lambda = 0.3f;
			component.m_cascadeEnd[x] = (lambda*cLog) + ((1 - lambda)*cUni);
		}

		component.m_data = m_shadowBuffer.newElement();
		component.m_data->data->Shadow_Spot = m_shadowCount;
		component.m_updateTime = 0.0f;
		component.m_shadowSpot = m_shadowCount;
		m_shadowCount += 4;
		m_shadowFBO.resize(m_shadowSize, m_shadowCount);
		// Default Values
		component.m_data->data->lightV = glm::mat4(1.0f);
		for (int x = 0; x < NUM_CASCADES; ++x) {
			component.m_data->data->lightVP[x] = glm::mat4(1.0f);
			component.m_data->data->inverseVP[x] = glm::inverse(glm::mat4(1.0f));
		}
	}
		

	// Public Attributes
	VectorBuffer<LightDirectional_Buffer> m_lightBuffer;
	VectorBuffer<LightDirectionalShadow_Buffer> m_shadowBuffer;

	
protected:
	// Protected Methods
	/** Render all the geometry from each light */
	void renderShadows(const float & deltaTime) {
		ECS & ecs = m_engine->getECS();
		glViewport(0, 0, m_shadowSize.x, m_shadowSize.y);
		m_shader_Shadow->bind();
		m_shadowFBO.bindForWriting();

		for each (const auto & pair in PQtoVector()) {
			const float clearDepth(1.0f);
			const glm::vec3 clear(0.0f);
			glUniform1i(0, pair.first->m_data->index);
			glUniform1i(1, pair.second->m_data->index);
			pair.second->m_data->wait();
			m_shadowFBO.clear(pair.second->m_shadowSpot);
			ecs.updateSystems(m_geometrySystems, deltaTime);
			pair.second->m_updateTime = glfwGetTime();
		}

		glViewport(0, 0, m_renderSize.x, m_renderSize.y);
	}
	/** Render all the lights */
	void renderLights(const float & deltaTime) {
		glEnable(GL_BLEND);
		glBlendEquation(GL_FUNC_ADD);
		glBlendFunc(GL_ONE, GL_ONE);
		glDisable(GL_DEPTH_TEST);
		glDepthMask(GL_FALSE);

		m_shader_Lighting->bind();										// Shader (directional)
		m_lightingFBO->bindForWriting();								// Ensure writing to lighting FBO
		m_geometryFBO->bindForReading();								// Read from Geometry FBO
		glBindTextureUnit(4, m_shadowFBO.m_textureIDS[2]);				// Shadow map (depth texture)
		m_visLights.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 3);		// SSBO visible light indices
		m_visShadows.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 4);		// SSBO visible shadow indices
		m_indirectShape.bindBuffer(GL_DRAW_INDIRECT_BUFFER);			// Draw call buffer
		glBindVertexArray(m_quadVAO);									// Quad VAO
		glDrawArraysIndirect(GL_TRIANGLES, 0);							// Now draw

		glDepthMask(GL_TRUE);
		glEnable(GL_DEPTH_TEST);
		glBlendFunc(GL_ONE, GL_ZERO);
		glDisable(GL_BLEND);
	}


private:
	// Private methods
	const std::vector<std::pair<LightDirectional_Component*, LightDirectionalShadow_Component*>> PQtoVector() const {
		PriorityList<float, std::pair<LightDirectional_Component*, LightDirectionalShadow_Component*>, std::greater<float>> m_closest(m_updateQuality / 2);
		std::vector<std::pair<LightDirectional_Component*, LightDirectionalShadow_Component*>> outList;
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
	Shared_Asset_Shader m_shader_Lighting, m_shader_Shadow, m_shader_Culling;
	Shared_Asset_Primitive m_shapeQuad;
	GLuint m_quadVAO;
	bool m_quadVAOLoaded;
	unsigned int m_updateQuality;
	glm::vec2 m_shadowSize;
	glm::ivec2	m_renderSize;
	StaticBuffer m_indirectShape;
	std::vector<GLuint> lightIndices, shadowIndices;
	DynamicBuffer m_visLights, m_visShadows;
	PriorityList<float, std::pair<LightDirectional_Component*, LightDirectionalShadow_Component*>, std::less<float>> m_oldest;
	ECSSystemList m_geometrySystems;
	FBO_Base * m_geometryFBO, * m_lightingFBO;
	FBO_Shadow_Directional m_shadowFBO;
	GLuint m_shadowCount = 0;
};

#endif // LIGHTINGDIRECTIONAL_S_H