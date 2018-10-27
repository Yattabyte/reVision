#pragma once
#ifndef LIGHTINGDIRECTIONAL_S_H
#define LIGHTINGDIRECTIONAL_S_H 

#include "Utilities\ECS\ecsSystem.h"
#include "Modules\Graphics\Components\LightDirectional_C.h"
#include "Utilities\GL\StaticBuffer.h"
#include "Utilities\GL\DynamicBuffer.h"
#include "Utilities\PriorityList.h"
#include "Engine.h"
#include <vector>


/** A struct that holds rendering data that can change frame-to-frame. */
struct Directional_RenderState {	
	GLint m_shadowCount = 0;
	glm::ivec2 m_shadowSize = glm::ivec2(1024);
	GLuint m_bounceSize = 16u,  m_updateQuality = 1u;
	float m_drawDistance = 1000.0f;
	StaticBuffer m_indirectShape = StaticBuffer(sizeof(GLuint) * 4), m_indirectBounce = StaticBuffer(sizeof(GLuint) * 4);
	DynamicBuffer m_visLights, m_visShadows;
	std::vector<std::pair<LightDirectional_Component*, LightDirectionalShadow_Component*>> m_shadowsToUpdate;
};

/** A system that updates the rendering state for directional lighting, using the ECS system. */
class LightDirectional_System : public BaseECSSystem {
public: 
	// (de)Constructors
	~LightDirectional_System() = default;
	LightDirectional_System(Engine * engine) : m_engine(engine) {
		// Declare component types used
		addComponentType(LightDirectional_Component::ID);
		addComponentType(LightDirectionalShadow_Component::ID, FLAG_OPTIONAL);
		GLuint data[] = { 0,0,0,0 };
		m_renderState.m_indirectShape.write(0, sizeof(GLuint) * 4, &data);
	}


	// Interface Implementation	
	virtual void updateComponents(const float & deltaTime, const std::vector<std::vector<BaseECSComponent*>> & components) override {
		// Accumulate Light Data		
		auto & graphics = m_engine->getGraphicsModule();
		const auto cameraBuffer = graphics.getActiveCameraBuffer();
		const glm::vec2 &size = cameraBuffer->data->Dimensions;
		const float ar = size.x / size.y;
		const float tanHalfHFOV = glm::radians(cameraBuffer->data->FOV) / 2.0f;
		const float tanHalfVFOV = atanf(tanf(tanHalfHFOV) / ar);
		const float near_plane = -CAMERA_NEAR_PLANE;
		const float far_plane = -m_renderState.m_drawDistance/2.0F;
		float cascadeEnd[NUM_CASCADES + 1];
		glm::vec3 middle[NUM_CASCADES], aabb[NUM_CASCADES];
		constexpr float lambda = 0.75f;
		cascadeEnd[0] = near_plane;
		for (int x = 1; x < NUM_CASCADES + 1; ++x) {
			const float xDivM = float(x) / float(NUM_CASCADES);
			const float cLog = near_plane * powf((far_plane / near_plane), xDivM);
			const float cUni = near_plane + (far_plane - near_plane) * xDivM;
			cascadeEnd[x] = (lambda * cLog) + (1.0f - lambda) * cUni;
		}
		for (int i = 0; i < NUM_CASCADES; i++) {
			float points[4] = {
				cascadeEnd[i] * tanHalfHFOV,
				cascadeEnd[i + 1] * tanHalfHFOV,
				cascadeEnd[i] * tanHalfVFOV,
				cascadeEnd[i + 1] * tanHalfVFOV
			};
			float maxCoord = std::max(abs(cascadeEnd[i]), abs(cascadeEnd[i + 1]));
			for (int x = 0; x < 4; ++x)
				maxCoord = std::max(maxCoord, abs(points[x]));
			// Find the middle of current view frustum chunk
			middle[i] = glm::vec3(0, 0, ((cascadeEnd[i + 1] - cascadeEnd[i]) / 2.0f) + cascadeEnd[i]);

			// Measure distance from middle to the furthest point of frustum slice
			// Use to make a bounding sphere, but then convert into a bounding box
			aabb[i] = glm::vec3(glm::distance(glm::vec3(maxCoord), middle[i]));
		}
		const glm::mat4 CamInv = glm::inverse(cameraBuffer->data->vMatrix);
		const glm::mat4 CamP = cameraBuffer->data->pMatrix;
		std::vector<GLint> lightIndices, shadowIndices;
		PriorityList<float, std::pair<LightDirectional_Component*, LightDirectionalShadow_Component*>, std::less<float>> oldest;
		for each (const auto & componentParam in components) {
			LightDirectional_Component * lightComponent = (LightDirectional_Component*)componentParam[0];
			LightDirectionalShadow_Component * shadowComponent = (LightDirectionalShadow_Component*)componentParam[1];
			lightIndices.push_back(lightComponent->m_data->index);
			if (shadowComponent) {
				shadowIndices.push_back(shadowComponent->m_data->index);
				oldest.insert(shadowComponent->m_updateTime, std::make_pair(lightComponent, shadowComponent));
				for (int i = 0; i < NUM_CASCADES; i++) {	
					const glm::vec3 volumeUnitSize = (aabb[i] - -aabb[i]) / (float)m_renderState.m_shadowSize.x;
					const glm::vec3 frustumpos = glm::vec3(shadowComponent->m_mMatrix * CamInv * glm::vec4(middle[i], 1.0f));
					const glm::vec3 clampedPos = glm::floor((frustumpos + (volumeUnitSize / 2.0f)) / volumeUnitSize) * volumeUnitSize;
					const glm::vec3 newMin = -aabb[i] + clampedPos;
					const glm::vec3 newMax = aabb[i] + clampedPos;
					const float l = newMin.x, r = newMax.x, b = newMax.y, t = newMin.y, n = -newMin.z, f = -newMax.z;
					const glm::mat4 pMatrix = glm::ortho(l, r, b, t, n, f);
					const glm::mat4 pvMatrix = pMatrix * shadowComponent->m_mMatrix;					
					shadowComponent->m_data->data->lightVP[i] = pvMatrix;
					shadowComponent->m_data->data->inverseVP[i] = inverse(pvMatrix);
					const glm::vec4 v1 = glm::vec4(0, 0, cascadeEnd[i + 1], 1.0f);
					const glm::vec4 v2 = CamP * v1;
					shadowComponent->m_data->data->CascadeEndClipSpace[i] = v2.z;
				}
			}
			else
				shadowIndices.push_back(-1);
		}

		// Update Draw Buffers
		const size_t & lightSize = lightIndices.size(), & shadowSize = shadowIndices.size();
		const GLuint bounceInstanceCount = GLuint(shadowSize * m_renderState.m_bounceSize);
		m_renderState.m_visLights.write(0, sizeof(GLuint) * lightSize, lightIndices.data());
		m_renderState.m_visShadows.write(0, sizeof(GLuint) * shadowSize, shadowIndices.data());
		m_renderState.m_indirectShape.write(sizeof(GLuint), sizeof(GLuint), &lightSize); // update primCount (2nd param)
		m_renderState.m_indirectBounce.write(sizeof(GLuint), sizeof(GLuint), &bounceInstanceCount);
		m_renderState.m_shadowsToUpdate = PQtoVector(oldest);
		m_renderState.m_shadowCount = (GLint)shadowSize;
	}
		

	// Public Attributes
	Directional_RenderState m_renderState;


private:
	// Private Methods
	/** Converts a priority queue into an stl vector.*/
	const std::vector<std::pair<LightDirectional_Component*, LightDirectionalShadow_Component*>> PQtoVector(PriorityList<float, std::pair<LightDirectional_Component*, LightDirectionalShadow_Component*>, std::less<float>> oldest) const {
		PriorityList<float, std::pair<LightDirectional_Component*, LightDirectionalShadow_Component*>, std::greater<float>> m_closest(m_renderState.m_updateQuality / 2);
		std::vector<std::pair<LightDirectional_Component*, LightDirectionalShadow_Component*>> outList;
		outList.reserve(m_renderState.m_updateQuality);

		for each (const auto &element in oldest.toList()) {
			if (outList.size() < (m_renderState.m_updateQuality / 2))
				outList.push_back(element);
			else
				m_closest.insert(element.second->m_updateTime, element);
		}

		for each (const auto &element in m_closest.toList()) {
			if (outList.size() >= m_renderState.m_updateQuality)
				break;
			outList.push_back(element);
		}

		return outList;
	}


	// Private Attributes
	Engine * m_engine = nullptr;
};

#endif // LIGHTINGDIRECTIONAL_S_H