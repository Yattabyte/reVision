#pragma once
#ifndef DIRECTIONALSYNC_SYSTEM_H
#define DIRECTIONALSYNC_SYSTEM_H

#include "Modules/World/ECS/ecsSystem.h"
#include "Modules/World/ECS/components.h"
#include "Modules/Graphics/Logical/components.h"
#include "Modules/Graphics/Lighting/components.h"
#include "Utilities/GL/GL_ArrayBuffer.h"
#include <memory>


/***/
struct Directional_Buffers {
	/** OpenGL buffer for directional lights. */
	struct Directional_Buffer {
		glm::mat4 lightV = glm::mat4(1.0f);
		glm::mat4 lightVP[NUM_CASCADES];
		glm::mat4 inverseVP[NUM_CASCADES];
		float CascadeEndClipSpace[NUM_CASCADES];
		glm::vec3 LightColor = glm::vec3(1.0f); float padding1;
		glm::vec3 LightDirection = glm::vec3(0, -1, 0); float padding2;
		float LightIntensity = 1.0f;
		int Shadow_Spot = -1; glm::vec2 padding3;
	};
	GL_ArrayBuffer<Directional_Buffer> lightBuffer;
	float shadowSize = 1.0f;
};

/***/
class DirectionalSync_System : public BaseECSSystem {
public:
	// Public (de)Constructors
	/***/
	inline ~DirectionalSync_System() = default;
	/***/
	inline DirectionalSync_System(const std::shared_ptr<Directional_Buffers> & buffers)
		: m_buffers(buffers) {
		addComponentType(Renderable_Component::ID, FLAG_REQUIRED);
		addComponentType(LightDirectional_Component::ID, FLAG_REQUIRED);
		addComponentType(LightColor_Component::ID, FLAG_OPTIONAL);
		addComponentType(Transform_Component::ID, FLAG_OPTIONAL);
		addComponentType(CameraFollower_Component::ID, FLAG_OPTIONAL);
	}


	// Public Interface Implementations
	inline virtual void updateComponents(const float & deltaTime, const std::vector<std::vector<BaseECSComponent*>> & components) override {
		for each (const auto & componentParam in components) {
			Renderable_Component * renderableComponent = (Renderable_Component*)componentParam[0];
			LightDirectional_Component * lightComponent = (LightDirectional_Component*)componentParam[1];
			LightColor_Component * colorComponent = (LightColor_Component*)componentParam[2];
			Transform_Component * transformComponent = (Transform_Component*)componentParam[3];
			CameraFollower_Component * camFollowComponent = (CameraFollower_Component*)componentParam[4];
			const auto & index = lightComponent->m_lightIndex;
			
			// Synchronize the component if it is visible
			if (renderableComponent->m_visible) {
				// Sync Camera Attributes
				if (camFollowComponent) {
					const auto size = glm::vec2(camFollowComponent->m_viewport->m_dimensions);
					const float ar = size.x / size.y;
					const float tanHalfHFOV = glm::radians(camFollowComponent->m_viewport->getFOV()) / 2.0f;
					const float tanHalfVFOV = atanf(tanf(tanHalfHFOV) / ar);
					const float near_plane = -CameraBuffer::BufferStructure::ConstNearPlane;
					const float far_plane = -camFollowComponent->m_viewport->getDrawDistance();
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
					const glm::mat4 CamInv = glm::inverse(camFollowComponent->m_viewport->getViewMatrix());
					const glm::mat4 CamP = camFollowComponent->m_viewport->getPerspectiveMatrix();
					const glm::mat4 sunModelMatrix = glm::inverse(glm::mat4_cast(transformComponent->m_transform.m_orientation) * glm::mat4_cast(glm::rotate(glm::quat(1, 0, 0, 0), glm::radians(90.0f), glm::vec3(0, 1.0f, 0))));					
					for (int i = 0; i < NUM_CASCADES; i++) {
						const glm::vec3 volumeUnitSize = (aabb[i] - -aabb[i]) / m_buffers->shadowSize;
						const glm::vec3 frustumpos = glm::vec3(sunModelMatrix * CamInv * glm::vec4(middle[i], 1.0f));
						const glm::vec3 clampedPos = glm::floor((frustumpos + (volumeUnitSize / 2.0f)) / volumeUnitSize) * volumeUnitSize;
						const glm::vec3 newMin = -aabb[i] + clampedPos;
						const glm::vec3 newMax = aabb[i] + clampedPos;
						const float l = newMin.x, r = newMax.x, b = newMax.y, t = newMin.y, n = -newMin.z, f = -newMax.z;
						const glm::mat4 pMatrix = glm::ortho(l, r, b, t, n, f);
						const glm::mat4 pvMatrix = pMatrix * sunModelMatrix;
						const glm::vec4 v1 = glm::vec4(0, 0, cascadeEnd[i + 1], 1.0f);
						const glm::vec4 v2 = CamP * v1;
						m_buffers->lightBuffer[index].lightVP[i] = pvMatrix;
						m_buffers->lightBuffer[index].inverseVP[i] = inverse(pvMatrix);
						m_buffers->lightBuffer[index].CascadeEndClipSpace[i] = v2.z;
					}					
				}

				// Sync Color Attributes
				if (colorComponent) {
					m_buffers->lightBuffer[index].LightColor = colorComponent->m_color;
					m_buffers->lightBuffer[index].LightIntensity = colorComponent->m_intensity;
				}
				
				// Sync Transform Attributes
				if (transformComponent) {
					const auto & orientation = transformComponent->m_transform.m_orientation;
					const auto matRot = glm::mat4_cast(orientation);
					const glm::mat4 sunTransform = matRot;
					m_buffers->lightBuffer[index].LightDirection = glm::vec3(glm::normalize(sunTransform * glm::vec4(1.0f, 0.0f, 0.0f, 0.0f)));

					if (lightComponent->m_hasShadow)
						m_buffers->lightBuffer[index].lightV = glm::inverse(sunTransform * glm::mat4_cast(glm::rotate(glm::quat(1, 0, 0, 0), glm::radians(90.0f), glm::vec3(0, 1.0f, 0))));
				}

				// Sync Buffer Attributes
				if (lightComponent->m_hasShadow)
					m_buffers->lightBuffer[index].Shadow_Spot = lightComponent->m_shadowSpot;
				else
					m_buffers->lightBuffer[index].Shadow_Spot = -1;
			}
		}
	}

private:
	// Private Attributes
	std::shared_ptr<Directional_Buffers> m_buffers;
};

#endif // DIRECTIONALSYNC_SYSTEM_H