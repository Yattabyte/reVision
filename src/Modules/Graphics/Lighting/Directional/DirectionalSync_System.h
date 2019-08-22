#pragma once
#ifndef DIRECTIONALSYNC_SYSTEM_H
#define DIRECTIONALSYNC_SYSTEM_H

#include "Modules/World/ECS/ecsSystem.h"
#include "Modules/World/ECS/components.h"
#include "Modules/Graphics/Lighting/Directional/DirectionalData.h"


/** An ECS system responsible for syncronizing directional lighting components and sending data to the GPU. */
class DirectionalSync_System : public BaseECSSystem {
public:
	// Public (de)Constructors
	/** Destroy this system. */
	inline ~DirectionalSync_System() = default;
	/** Construct this system.
	@param	frameData	shared pointer of common data that changes frame-to-frame. */
	inline DirectionalSync_System(const std::shared_ptr<DirectionalData> & frameData)
		: m_frameData(frameData) {
		addComponentType(LightDirectional_Component::ID, FLAG_REQUIRED);
		addComponentType(Shadow_Component::ID, FLAG_REQUIRED);
		addComponentType(CameraArray_Component::ID, FLAG_REQUIRED);
		addComponentType(LightColor_Component::ID, FLAG_OPTIONAL);
		addComponentType(Transform_Component::ID, FLAG_OPTIONAL);
	}


	// Public Interface Implementations
	inline virtual void updateComponents(const float & deltaTime, const std::vector<std::vector<BaseECSComponent*>> & components) override {
		// Resize light buffers to match number of entities this frame
		m_frameData->lightBuffer.resize(components.size());
		m_frameData->lightBuffer.beginWriting();
		int index = 0;
		for each (const auto & componentParam in components) {
			auto* lightComponent = (LightDirectional_Component*)componentParam[0];
			auto* shadowComponent = (Shadow_Component*)componentParam[1];
			auto* cameraComponent = (CameraArray_Component*)componentParam[2];
			auto* colorComponent = (LightColor_Component*)componentParam[3];
			auto* transformComponent = (Transform_Component*)componentParam[4];

			// Synchronize the component if it is visible
				// Sync Camera Attributes
			if (cameraComponent) {
				cameraComponent->m_cameras.resize(NUM_CASCADES);
				cameraComponent->m_updateTimes.resize(NUM_CASCADES);
				const auto size = glm::vec2(m_frameData->clientCamera.get()->get()->Dimensions);
				const float ar = size.x / size.y;
				const float tanHalfHFOV = glm::radians(m_frameData->clientCamera.get()->get()->FOV) / 2.0f;
				const float tanHalfVFOV = atanf(tanf(tanHalfHFOV) / ar);
				const float near_plane = -Camera::ConstNearPlane;
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
				const glm::mat4 CamInv = m_frameData->clientCamera.get()->get()->vMatrixInverse;
				const glm::mat4 CamP = m_frameData->clientCamera.get()->get()->pMatrix;
				const glm::mat4 sunModelMatrix = glm::inverse(glm::mat4_cast(transformComponent->m_worldTransform.m_orientation) * glm::mat4_cast(glm::rotate(glm::quat(1, 0, 0, 0), glm::radians(90.0f), glm::vec3(0, 1.0f, 0))));
				const glm::mat4 sunInverse = glm::inverse(sunModelMatrix);
				for (int i = 0; i < NUM_CASCADES; ++i) {
					auto& cam = cameraComponent->m_cameras[i];
					auto& camData = *cam.get();
					const glm::vec3 volumeUnitSize = (aabb[i] - -aabb[i]) / m_frameData->shadowData->shadowSize;
					const glm::vec3 frustumpos = glm::vec3(sunModelMatrix * CamInv * glm::vec4(middle[i], 1.0f));
					const glm::vec3 clampedPos = glm::floor((frustumpos + (volumeUnitSize / 2.0f)) / volumeUnitSize) * volumeUnitSize;
					const glm::vec3 newMin = -aabb[i] + clampedPos;
					const glm::vec3 newMax = aabb[i] + clampedPos;
					const float l = newMin.x, r = newMax.x, b = newMax.y, t = newMin.y, n = -newMin.z, f = -newMax.z;
					const glm::mat4 pMatrix = glm::ortho(l, r, b, t, n, f);
					const glm::mat4 pvMatrix = pMatrix * sunModelMatrix;
					const glm::vec4 v_near = CamP * glm::vec4(0, 0, cascadeEnd[i], 1.0f);
					const glm::vec4 v_far = CamP * glm::vec4(0, 0, cascadeEnd[i + 1], 1.0f);
					camData.Dimensions = glm::ivec2((int)m_frameData->shadowData->shadowSize);
					camData.FOV = 90.0f;
					camData.NearPlane = v_near.z;
					camData.FarPlane = v_far.z;
					auto pos = CamInv * glm::vec4(0, 0, -v_far.z / 2.0f, 1.0f);
					pos /= pos.w;
					camData.EyePosition = glm::vec3(pos);
					camData.pMatrix = pMatrix;
					camData.pMatrixInverse = glm::inverse(pMatrix);
					camData.vMatrix = sunModelMatrix;
					camData.vMatrixInverse = sunInverse;
					camData.pvMatrix = pMatrix * sunModelMatrix;
					cam.updateFrustum();
					if (cam.getEnabled()) {
						lightComponent->m_pvMatrices[i] = pvMatrix;
						lightComponent->m_cascadeEnds[i] = v_far.z;
					}
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
				const auto& orientation = transformComponent->m_worldTransform.m_orientation;
				const auto matRot = glm::mat4_cast(orientation);
				const glm::mat4 sunTransform = matRot;
				m_frameData->lightBuffer[index].LightDirection = glm::vec3(glm::normalize(sunTransform * glm::vec4(1.0f, 0.0f, 0.0f, 0.0f)));
			}

			// Sync Buffer Attributes
			m_frameData->lightBuffer[index].Shadow_Spot = shadowComponent->m_shadowSpot;
		}
		index++;		
	}


private:
	// Private Attributes
	std::shared_ptr<DirectionalData> m_frameData;
};

#endif // DIRECTIONALSYNC_SYSTEM_H