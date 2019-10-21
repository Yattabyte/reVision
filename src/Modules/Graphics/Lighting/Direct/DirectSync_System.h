#pragma once
#ifndef DIRECTSYNC_SYSTEM_H
#define DIRECTSYNC_SYSTEM_H

#include "Modules/ECS/ecsSystem.h"
#include "Modules/ECS/component_types.h"
#include "Modules/Graphics/Lighting/Direct/DirectData.h"
#include <algorithm>


/** An ECS system responsible for synchronizing light components and sending data to the GPU. */
class DirectSync_System final : public ecsBaseSystem {
public:
	// Public (de)Constructors
	/** Destroy this system. */
	inline ~DirectSync_System() = default;
	/** Construct this system.
	@param	frameData	shared pointer of common data that changes frame-to-frame. */
	inline DirectSync_System(const std::shared_ptr<Direct_Light_Data>& frameData)
		: m_frameData(frameData) {
		addComponentType(Transform_Component::m_ID, FLAG_REQUIRED);
		addComponentType(Light_Component::m_ID, FLAG_REQUIRED);
		addComponentType(Shadow_Component::m_ID, FLAG_OPTIONAL);
	}


	// Public Interface Implementations
	inline virtual void updateComponents(const float& deltaTime, const std::vector<std::vector<ecsBaseComponent*>>& components) override final {
		// Resize light buffers to match number of entities this frame
		m_frameData->lightBuffer.resize(components.size());
		m_frameData->lightBuffer.beginWriting();
		int index = 0;
		for each (const auto & componentParam in components) {
			auto* trans = (Transform_Component*)componentParam[0];
			auto* light = (Light_Component*)componentParam[1];
			auto* shadow = (Shadow_Component*)componentParam[2];

			// Sync Common Buffer Attributes
			const auto radiusSquared = (light->m_radius * light->m_radius);
			const auto& position = trans->m_worldTransform.m_position;
			const auto transM = glm::translate(glm::mat4(1.0f), position);
			const auto rotM = glm::mat4_cast(trans->m_worldTransform.m_orientation);
			const auto sclM = glm::scale(glm::mat4(1.0f), glm::vec3(radiusSquared * 1.1f));
			trans->m_localTransform.m_scale = glm::vec3(radiusSquared * 1.1f);
			trans->m_localTransform.update();
			m_frameData->lightBuffer[index].mMatrix = transM * rotM * sclM;
			m_frameData->lightBuffer[index].LightColor = light->m_color;
			m_frameData->lightBuffer[index].LightPosition = trans->m_worldTransform.m_position;
			m_frameData->lightBuffer[index].LightDirection = glm::normalize(rotM * glm::vec4(0.0f, 0.0f, 1.0f, 0.0f));
			m_frameData->lightBuffer[index].LightIntensity = light->m_intensity;
			m_frameData->lightBuffer[index].LightRadius = light->m_radius;
			m_frameData->lightBuffer[index].LightCutoff = cosf(glm::radians(light->m_cutoff / 2.0f));
			m_frameData->lightBuffer[index].Shadow_Spot = shadow ? shadow->m_shadowSpot : -1;
			m_frameData->lightBuffer[index].Light_Type = static_cast<int>(light->m_type);

			// Sync Shadow Attributes
			if (shadow) {
				const auto updateCamera = [&](Camera& camera, const glm::mat4& pMatrix, const glm::mat4& vMatrix, const glm::vec3& position, const float& nearPlane = -Camera::ConstNearPlane, const float& farPlane = 1.0f, const float& fov = 90.0f) {
					auto& camData = *camera.get();
					camData.pMatrix = pMatrix;
					camData.pMatrixInverse = glm::inverse(pMatrix);
					camData.vMatrix = vMatrix;
					camData.vMatrixInverse = glm::inverse(vMatrix);
					camData.pvMatrix = pMatrix * vMatrix;
					camData.EyePosition = position;
					camData.Dimensions = glm::ivec2((int)m_frameData->shadowData->shadowSize);
					camData.NearPlane = nearPlane;
					camData.FarPlane = farPlane;
					camData.FOV = fov;
					camera.updateFrustum();
				};
				if (light->m_type == Light_Component::Light_Type::DIRECTIONAL) {
					constexpr auto NUM_CASCADES = 4;
					shadow->m_cameras.resize(4);
					const auto size = m_frameData->clientCamera->get()->Dimensions;
					const auto ar = size.x / size.y;
					const auto tanHalfHFOV = glm::radians(m_frameData->clientCamera->get()->FOV) / 2.0f;
					const auto tanHalfVFOV = atanf(tanf(tanHalfHFOV) / ar);
					const auto near_plane = -Camera::ConstNearPlane;
					const auto far_plane = -m_frameData->clientCamera->get()->FarPlane;
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
					const auto CamInv = m_frameData->clientCamera->get()->vMatrixInverse;
					const auto CamP = m_frameData->clientCamera->get()->pMatrix;
					const auto sunModelMatrix = glm::inverse(glm::mat4_cast(trans->m_worldTransform.m_orientation) * glm::mat4_cast(glm::rotate(glm::quat(1, 0, 0, 0), glm::radians(180.0f), glm::vec3(0, 1.0f, 0))));
					for (int x = 0; x < NUM_CASCADES; ++x) {
						const glm::vec3 volumeUnitSize = (aabb[x] - -aabb[x]) / m_frameData->shadowData->shadowSize;
						const glm::vec3 frustumpos = glm::vec3(sunModelMatrix * CamInv * glm::vec4(middle[x], 1.0f));
						const glm::vec3 clampedPos = glm::floor((frustumpos + (volumeUnitSize / 2.0f)) / volumeUnitSize) * volumeUnitSize;
						const glm::vec3 newMin = -aabb[x] + clampedPos;
						const glm::vec3 newMax = aabb[x] + clampedPos;
						const float l = newMin.x, r = newMax.x, b = newMax.y, t = newMin.y, n = -newMin.z, f = -newMax.z;
						const glm::mat4 pMatrix = glm::ortho(l, r, b, t, n, f);
						const glm::vec4 v_near = CamP * glm::vec4(0, 0, cascadeEnd[x], 1.0f);
						const glm::vec4 v_far = CamP * glm::vec4(0, 0, cascadeEnd[x + 1], 1.0f);
						auto pos = CamInv * glm::vec4(0, 0, -v_far.z / 2.0f, 1.0f);
						pos /= pos.w;

						updateCamera(shadow->m_cameras[x], pMatrix, sunModelMatrix, glm::vec3(pos), v_near.z, v_far.z, 180.0f);
						if (shadow->m_cameras[x].getEnabled()) {
							m_frameData->lightBuffer[index].lightVP[x] = shadow->m_cameras[x].get()->pvMatrix;
							m_frameData->lightBuffer[index].CascadeEndClipSpace[x] = v_far.z;
						}
					}
				}
				else if (light->m_type == Light_Component::Light_Type::POINT) {
					shadow->m_cameras.resize(6);
					const auto pMatrix = glm::perspective(glm::radians(90.0f), 1.0f, 0.01f, radiusSquared);
					const glm::mat4 vMatrices[6] = {
						glm::lookAt(position, position + glm::vec3(1, 0, 0), glm::vec3(0, -1, 0)),
						glm::lookAt(position, position + glm::vec3(-1, 0, 0), glm::vec3(0, -1, 0)),
						glm::lookAt(position, position + glm::vec3(0, 1, 0), glm::vec3(0, 0, 1)),
						glm::lookAt(position, position + glm::vec3(0, -1, 0), glm::vec3(0, 0, -1)),
						glm::lookAt(position, position + glm::vec3(0, 0, 1), glm::vec3(0, -1, 0)),
						glm::lookAt(position, position + glm::vec3(0, 0, -1), glm::vec3(0, -1, 0))
					};
					for (int x = 0; x < 6; ++x) {
						updateCamera(shadow->m_cameras[x], pMatrix, vMatrices[x], position, -Camera::ConstNearPlane, radiusSquared, 90.0f);
						m_frameData->lightBuffer[index].lightVP[x] = shadow->m_cameras[x].get()->pvMatrix;
					}
				}
				else if (light->m_type == Light_Component::Light_Type::SPOT) {
					shadow->m_cameras.resize(1);
					const auto pMatrix = glm::perspective(glm::radians(light->m_cutoff), 1.0f, 0.01f, radiusSquared);
					const auto vMatrix = glm::inverse(transM * rotM);
					updateCamera(shadow->m_cameras[0], pMatrix, vMatrix, position, -Camera::ConstNearPlane, radiusSquared, light->m_cutoff);
					m_frameData->lightBuffer[index].lightVP[0] = shadow->m_cameras[0].get()->pvMatrix;
				}
				shadow->m_updateTimes.resize(shadow->m_cameras.size());
			}
			index++;
		}
		m_frameData->lightBuffer.endWriting();
	}


private:
	// Private Attributes
	std::shared_ptr<Direct_Light_Data> m_frameData;
};

#endif // DIRECTSYNC_SYSTEM_H