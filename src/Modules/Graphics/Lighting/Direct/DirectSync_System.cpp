#include "Modules/Graphics/Lighting/Direct/DirectSync_System.h"
#include "Modules/Graphics/Lighting/Direct/DirectData.h"
#include "Modules/ECS/component_types.h"
#include "glm/gtx/component_wise.hpp"
#include <algorithm>


DirectSync_System::DirectSync_System(Direct_Light_Data& frameData) noexcept :
	m_frameData(frameData)
{
	addComponentType(Transform_Component::Runtime_ID, RequirementsFlag::FLAG_REQUIRED);
	addComponentType(Light_Component::Runtime_ID, RequirementsFlag::FLAG_REQUIRED);
	addComponentType(Shadow_Component::Runtime_ID, RequirementsFlag::FLAG_OPTIONAL);
}

void DirectSync_System::updateComponents(const float& deltaTime, const std::vector<std::vector<ecsBaseComponent*>>& components) noexcept 
{
	// Resize light buffers to match number of entities this frame
	m_frameData.lightBuffer.resize(components.size());
	m_frameData.lightBuffer.beginWriting();
	int index = 0;
	for (const auto& componentParam : components) {
		auto* trans = static_cast<Transform_Component*>(componentParam[0]);
		auto* light = static_cast<Light_Component*>(componentParam[1]);
		auto* shadow = static_cast<Shadow_Component*>(componentParam[2]);

		// Sync Common Buffer Attributes
		const auto radiusSquared = (light->m_radius * light->m_radius);
		const auto& position = trans->m_worldTransform.m_position;
		const auto transM = glm::translate(glm::mat4(1.0f), position);
		const auto rotM = glm::mat4_cast(trans->m_worldTransform.m_orientation);
		const auto sclM = glm::scale(glm::mat4(1.0f), glm::vec3(radiusSquared * 1.1f));
		trans->m_localTransform.m_scale = glm::vec3(radiusSquared * 1.1f);
		trans->m_localTransform.update();
		m_frameData.lightBuffer[index].mMatrix = transM * rotM * sclM;
		m_frameData.lightBuffer[index].LightColor = light->m_color;
		m_frameData.lightBuffer[index].LightPosition = trans->m_worldTransform.m_position;
		m_frameData.lightBuffer[index].LightDirection = glm::normalize(rotM * glm::vec4(0.0f, 0.0f, 1.0f, 0.0f));
		m_frameData.lightBuffer[index].LightIntensity = light->m_intensity;
		m_frameData.lightBuffer[index].LightRadius = light->m_radius;
		m_frameData.lightBuffer[index].LightCutoff = cosf(glm::radians(light->m_cutoff / 2.0f));
		m_frameData.lightBuffer[index].Shadow_Spot = shadow ? shadow->m_shadowSpot : -1;
		m_frameData.lightBuffer[index].Light_Type = static_cast<int>(light->m_type);

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
				camData.Dimensions = glm::ivec2((int)m_frameData.shadowData.shadowSize);
				camData.NearPlane = nearPlane;
				camData.FarPlane = farPlane;
				camData.FOV = fov;
				camera.updateFrustum();
			};
			if (light->m_type == Light_Component::Light_Type::DIRECTIONAL) {
				constexpr auto NUM_CASCADES = 4;
				shadow->m_cameras.resize(NUM_CASCADES);
				const auto& ClientCamera = *m_frameData.clientCamera.get();
				const auto& CamInv = ClientCamera.vMatrixInverse;
				const auto& CamP = ClientCamera.pMatrix;
				const auto& size = ClientCamera.Dimensions;
				const auto sunModelMatrix = glm::inverse(glm::mat4_cast(trans->m_worldTransform.m_orientation) * glm::mat4_cast(glm::rotate(glm::quat(1, 0, 0, 0), glm::radians(180.0f), glm::vec3(0, 1.0f, 0))));
				const auto ar = size.x / size.y;
				const auto tanHalfHFOV = glm::radians(ClientCamera.FOV) / 2.0f;
				const auto tanHalfVFOV = atanf(tanf(tanHalfHFOV) / ar);
				const auto near_plane = -Camera::ConstNearPlane;
				const auto far_plane = -std::min(light->m_radius * light->m_radius, ClientCamera.FarPlane);
				float cascadeEnd[NUM_CASCADES + 1];
				constexpr float lambda = 0.75f;
				cascadeEnd[0] = near_plane;
				for (int x = 1; x < NUM_CASCADES + 1; ++x) {
					const float xDivM = float(x) / float(NUM_CASCADES);
					const float cLog = near_plane * powf((far_plane / near_plane), xDivM);
					const float cUni = near_plane + (far_plane - near_plane) * xDivM;
					cascadeEnd[x] = (lambda * cLog) + (1.0f - lambda) * cUni;
				}
				for (int x = 0; x < NUM_CASCADES; x++) {
					// Find the middle of current view frustum chunk
					const auto middle = glm::vec3(0, 0, ((cascadeEnd[x + 1] - cascadeEnd[x]) / 2.0f) + cascadeEnd[x]);

					// Measure distance from middle to the furthest point of frustum slice
					// Use to make a bounding sphere, but then convert into a bounding box
					const auto points = glm::vec4(
						cascadeEnd[x] * tanHalfHFOV,
						cascadeEnd[x + 1] * tanHalfHFOV,
						cascadeEnd[x] * tanHalfVFOV,
						cascadeEnd[x + 1] * tanHalfVFOV
					);
					const auto maxCoord = std::max(std::max(abs(cascadeEnd[x]), abs(cascadeEnd[x + 1])), glm::compMax(points));
					const auto aabb = glm::vec3(glm::distance(glm::vec3(maxCoord), middle));

					// Calculate orthographic projection variables
					const auto volumeUnitSize = (aabb - -aabb) / m_frameData.shadowData.shadowSize;
					const auto frustumpos = glm::vec3(sunModelMatrix * CamInv * glm::vec4(middle, 1.0f));
					const auto clampedPos = glm::floor((frustumpos + (volumeUnitSize / 2.0f)) / volumeUnitSize) * volumeUnitSize;
					const auto newMin = -aabb + clampedPos;
					const auto newMax = aabb + clampedPos;
					const auto l = newMin.x, r = newMax.x, b = newMax.y, t = newMin.y, n = -newMin.z, f = -newMax.z;
					const auto pMatrix = glm::ortho(l, r, b, t, n, f);
					const auto v_near = CamP * glm::vec4(0, 0, cascadeEnd[x], 1.0f);
					const auto v_far = CamP * glm::vec4(0, 0, cascadeEnd[x + 1], 1.0f);
					const auto pos = CamInv * glm::vec4(0, 0, -v_far.z / 2.0f, 1.0f);

					updateCamera(shadow->m_cameras[x], pMatrix, sunModelMatrix, glm::vec3(pos / pos.w), v_near.z, v_far.z, 180.0f);
					if (shadow->m_cameras[x].getEnabled()) {
						m_frameData.lightBuffer[index].lightVP[x] = shadow->m_cameras[x].get()->pvMatrix;
						m_frameData.lightBuffer[index].CascadeEndClipSpace[x] = v_far.z;
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
					if (shadow->m_cameras[x].getEnabled())
						m_frameData.lightBuffer[index].lightVP[x] = shadow->m_cameras[x].get()->pvMatrix;
				}
			}
			else if (light->m_type == Light_Component::Light_Type::SPOT) {
				shadow->m_cameras.resize(1);
				const auto pMatrix = glm::perspective(glm::radians(light->m_cutoff), 1.0f, 0.01f, radiusSquared);
				const auto vMatrix = glm::inverse(transM * rotM);
				updateCamera(shadow->m_cameras[0], pMatrix, vMatrix, position, -Camera::ConstNearPlane, radiusSquared, light->m_cutoff);
				if (shadow->m_cameras[0].getEnabled())
					m_frameData.lightBuffer[index].lightVP[0] = shadow->m_cameras[0].get()->pvMatrix;
			}
			shadow->m_updateTimes.resize(shadow->m_cameras.size());
		}
		index++;
	}
	m_frameData.lightBuffer.endWriting();
}