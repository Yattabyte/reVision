#pragma once
#ifndef DIRECTIONALSYNC_SYSTEM_H
#define DIRECTIONALSYNC_SYSTEM_H

#include "Modules/World/ECS/ecsSystem.h"
#include "Modules/World/ECS/components.h"
#include "Modules/Graphics/Logical/components.h"
#include "Modules/Graphics/Lighting/components.h"
#include "Modules/Graphics/Lighting/Directional/DirectionalData.h"


/***/
class DirectionalSync_System : public BaseECSSystem {
public:
	// Public (de)Constructors
	/***/
	inline ~DirectionalSync_System() = default;
	/***/
	inline DirectionalSync_System(const std::shared_ptr<DirectionalData> & frameData, const std::shared_ptr<std::vector<CameraBuffer::CamStruct*>> & cameras)
		: m_frameData(frameData), m_cameras(cameras) {
		addComponentType(Renderable_Component::ID, FLAG_REQUIRED);
		addComponentType(LightDirectional_Component::ID, FLAG_REQUIRED);
		addComponentType(Shadow_Component::ID, FLAG_REQUIRED);
		addComponentType(CameraArray_Component::ID, FLAG_REQUIRED);
		addComponentType(LightColor_Component::ID, FLAG_OPTIONAL);
		addComponentType(Transform_Component::ID, FLAG_OPTIONAL);
	}


	// Public Interface Implementations
	inline virtual void updateComponents(const float & deltaTime, const std::vector<std::vector<BaseECSComponent*>> & components) override {
		// Link together the dimensions of view info to that of the viewport vectors
		m_frameData->viewInfo.resize(m_cameras->size());

		// Resize light buffers to match number of entities this frame
		m_frameData->lightBuffer.resize(components.size());
		m_frameData->lightBuffer.beginWriting();
		int index = 0;
		for each (const auto & componentParam in components) {
			Renderable_Component * renderableComponent = (Renderable_Component*)componentParam[0];
			LightDirectional_Component * lightComponent = (LightDirectional_Component*)componentParam[1];
			Shadow_Component * shadowComponent = (Shadow_Component*)componentParam[2];
			CameraArray_Component * cameraComponent = (CameraArray_Component*)componentParam[3];
			LightColor_Component * colorComponent = (LightColor_Component*)componentParam[4];
			Transform_Component * transformComponent = (Transform_Component*)componentParam[5];

			// Synchronize the component if it is visible
			if (renderableComponent->m_visibleAtAll) {
				// Sync Camera Attributes
				if (cameraComponent) {
					cameraComponent->m_cameras.resize(NUM_CASCADES);
					if (shadowComponent->m_outOfDate) {
						const auto size = glm::vec2(m_frameData->clientCamera.get()->get()->Dimensions);
						const float ar = size.x / size.y;
						const float tanHalfHFOV = glm::radians(m_frameData->clientCamera.get()->get()->FOV) / 2.0f;
						const float tanHalfVFOV = atanf(tanf(tanHalfHFOV) / ar);
						const float near_plane = -CameraBuffer::ConstNearPlane;
						const float far_plane = -m_frameData->clientCamera.get()->get()->FarPlane;
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
						const glm::mat4 CamInv = glm::inverse(m_frameData->clientCamera.get()->get()->vMatrix);
						const glm::mat4 CamP = m_frameData->clientCamera.get()->get()->pMatrix;
						const glm::mat4 sunModelMatrix = glm::inverse(glm::mat4_cast(transformComponent->m_transform.m_orientation) * glm::mat4_cast(glm::rotate(glm::quat(1, 0, 0, 0), glm::radians(90.0f), glm::vec3(0, 1.0f, 0))));
						for (int i = 0; i < NUM_CASCADES; ++i) {
							const glm::vec3 volumeUnitSize = (aabb[i] - -aabb[i]) / m_frameData->shadowData->shadowSize;
							const glm::vec3 frustumpos = glm::vec3(sunModelMatrix * CamInv * glm::vec4(middle[i], 1.0f));
							const glm::vec3 clampedPos = glm::floor((frustumpos + (volumeUnitSize / 2.0f)) / volumeUnitSize) * volumeUnitSize;
							const glm::vec3 newMin = -aabb[i] + clampedPos;
							const glm::vec3 newMax = aabb[i] + clampedPos;
							const float l = newMin.x, r = newMax.x, b = newMax.y, t = newMin.y, n = -newMin.z, f = -newMax.z;
							const glm::mat4 pMatrix = glm::ortho(l, r, b, t, n, f);
							const glm::mat4 pvMatrix = pMatrix * sunModelMatrix;
							const glm::vec4 v1 = glm::vec4(0, 0, cascadeEnd[i + 1], 1.0f);
							const glm::vec4 v2 = CamP * v1;
							lightComponent->m_pvMatrices[i] = pvMatrix;
							lightComponent->m_cascadeEnds[i] = v2.z;
							auto & cam = cameraComponent->m_cameras[i];
							cam.Dimensions = glm::ivec2(m_frameData->shadowData->shadowSize);
							cam.FOV = 90.0f;
							cam.NearPlane = newMin.z;
							cam.FarPlane = newMax.z;
							cam.EyePosition = clampedPos;
							cam.pMatrix = pMatrix;
							cam.pMatrixInverse = glm::inverse(pMatrix);
							cam.vMatrix = sunModelMatrix;
							cam.vMatrixInverse = glm::inverse(sunModelMatrix);
							cam.pvMatrix = pMatrix * sunModelMatrix;
						}
						shadowComponent->m_outOfDate = false;
					}
					for (int x = 0; x < NUM_CASCADES; ++x) {
						m_frameData->lightBuffer[index].lightVP[x] = lightComponent->m_pvMatrices[x];
						m_frameData->lightBuffer[index].CascadeEndClipSpace[x] = lightComponent->m_cascadeEnds[x];
					}
				}

				// Sync Color Attributes
				if (colorComponent) {
					m_frameData->lightBuffer[index].LightColor = colorComponent->m_color;
					m_frameData->lightBuffer[index].LightIntensity = colorComponent->m_intensity;
				}

				// Sync Transform Attributes
				if (transformComponent) {
					const auto & orientation = transformComponent->m_transform.m_orientation;
					const auto matRot = glm::mat4_cast(orientation);
					const glm::mat4 sunTransform = matRot;
					m_frameData->lightBuffer[index].LightDirection = glm::vec3(glm::normalize(sunTransform * glm::vec4(1.0f, 0.0f, 0.0f, 0.0f)));
				}

				// Sync Buffer Attributes
				m_frameData->lightBuffer[index].Shadow_Spot = shadowComponent->m_shadowSpot;
			}
			index++;
		}
	}


private:
	// Private Attributes
	std::shared_ptr<DirectionalData> m_frameData;
	std::shared_ptr<std::vector<CameraBuffer::CamStruct*>> m_cameras;
};

#endif // DIRECTIONALSYNC_SYSTEM_H