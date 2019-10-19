#pragma once
#ifndef SPOTSYNC_SYSTEM_H
#define SPOTSYNC_SYSTEM_H

#include "Modules/ECS/ecsSystem.h"
#include "Modules/ECS/component_types.h"
#include "Modules/Graphics/Lighting/Point/PointData.h"


/** An ECS system responsible for syncronizing spot lighting components and sending data to the GPU. */
class SpotSync_System final : public ecsBaseSystem {
public:
	// Public (de)Constructors
	/** Destroy this system. */
	inline ~SpotSync_System() = default;
	/** Construct this system.
	@param	frameData	shared pointer of common data that changes frame-to-frame. */
	inline SpotSync_System(const std::shared_ptr<SpotData>& frameData)
		: m_frameData(frameData) {
		addComponentType(LightSpot_Component::m_ID, FLAG_REQUIRED);
		addComponentType(LightColor_Component::m_ID, FLAG_OPTIONAL);
		addComponentType(LightRadius_Component::m_ID, FLAG_OPTIONAL);
		addComponentType(LightCutoff_Component::m_ID, FLAG_OPTIONAL);
		addComponentType(Transform_Component::m_ID, FLAG_OPTIONAL);
		addComponentType(Shadow_Component::m_ID, FLAG_OPTIONAL);
		addComponentType(Camera_Component::m_ID, FLAG_OPTIONAL);
		addComponentType(BoundingSphere_Component::m_ID, FLAG_OPTIONAL);
	}


	// Public Interface Implementations
	inline virtual void updateComponents(const float& deltaTime, const std::vector<std::vector<ecsBaseComponent*>>& components) override final {
		// Resize light buffers to match number of entities this frame
		m_frameData->lightBuffer.resize(components.size());
		m_frameData->lightBuffer.beginWriting();
		int index = 0;
		for each (const auto & componentParam in components) {
			auto* lightComponent = (LightSpot_Component*)componentParam[0];
			auto* colorComponent = (LightColor_Component*)componentParam[1];
			auto* radiusComponent = (LightRadius_Component*)componentParam[2];
			auto* cutoffComponent = (LightCutoff_Component*)componentParam[3];
			auto* transformComponent = (Transform_Component*)componentParam[4];
			auto* shadowComponent = (Shadow_Component*)componentParam[5];
			auto* cameraComponent = (Camera_Component*)componentParam[6];
			auto* bsphereComponent = (BoundingSphere_Component*)componentParam[7];

			// Sync Color Attributes
			if (colorComponent) {
				m_frameData->lightBuffer[index].LightColor = colorComponent->m_color;
				m_frameData->lightBuffer[index].LightIntensity = colorComponent->m_intensity;
			}

			// Sync Radius Attributes
			float radiusSquared = 1.0f;
			if (radiusComponent) {
				m_frameData->lightBuffer[index].LightRadius = radiusComponent->m_radius;
				radiusSquared = radiusComponent->m_radius * radiusComponent->m_radius;
			}
			if (bsphereComponent)
				bsphereComponent->m_radius = radiusSquared;

			// Sync Cutoff Attributes
			float cutoff = 90.0f;
			if (cutoffComponent) {
				m_frameData->lightBuffer[index].LightCutoff = cosf(glm::radians(cutoffComponent->m_cutoff));
				cutoff = cutoffComponent->m_cutoff;
			}

			// Sync Transform Attributes
			if (transformComponent) {
				const auto& position = transformComponent->m_worldTransform.m_position;
				const auto& orientation = transformComponent->m_worldTransform.m_orientation;
				const auto matRot = glm::mat4_cast(orientation);
				m_frameData->lightBuffer[index].LightPosition = position;
				auto dir = matRot * glm::vec4(0, 0, -1, 1);
				dir /= dir.w;
				m_frameData->lightBuffer[index].LightDirection = glm::normalize(glm::vec3(dir));
				const glm::mat4 trans = glm::translate(glm::mat4(1.0f), position);
				const glm::mat4 scl = glm::scale(glm::mat4(1.0f), glm::vec3(radiusSquared) * 1.1f);
				m_frameData->lightBuffer[index].mMatrix = trans * matRot * scl;

				const glm::mat4 pMatrix = glm::perspective(glm::radians(cutoff * 2.0f), 1.0f, 0.01f, radiusSquared);
				const glm::mat4 pMatrixInverse = glm::inverse(pMatrix);
				const auto pv = pMatrix * glm::inverse(trans * glm::mat4_cast(orientation));
				m_frameData->lightBuffer[index].lightPV = pv;
				if (cameraComponent) {
					auto& camData = *cameraComponent->m_camera.get();
					camData.Dimensions = glm::ivec2((int)m_frameData->shadowData->shadowSize);
					camData.FarPlane = radiusSquared;
					camData.EyePosition = position;
					camData.FOV = cutoff * 2.0f;
					camData.pMatrix = pMatrix;
					camData.pMatrixInverse = pMatrixInverse;
					camData.vMatrix = glm::inverse(trans * glm::mat4_cast(orientation));
					camData.vMatrixInverse = (trans * glm::mat4_cast(orientation)); // yes, this is correct
					camData.pvMatrix = pMatrix * camData.vMatrix;
					cameraComponent->m_camera.updateFrustum();
				}
			}

			// Sync Buffer Attributes
			m_frameData->lightBuffer[index].Shadow_Spot = shadowComponent ? shadowComponent->m_shadowSpot : -1;
			index++;
		}
		m_frameData->lightBuffer.endWriting();
	}


private:
	// Private Attributes
	std::shared_ptr<SpotData> m_frameData;
};

#endif // SPOTSYNC_SYSTEM_H