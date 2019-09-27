#pragma once
#ifndef POINTSYNC_SYSTEM_H
#define POINTSYNC_SYSTEM_H

#include "Modules/ECS/ecsSystem.h"
#include "Modules/ECS/component_types.h"
#include "Modules/Graphics/Lighting/Point/PointData.h"


/** An ECS system responsible for syncronizing point lighting components and sending data to the GPU. */
class PointSync_System final : public ecsBaseSystem {
public:
	// Public (de)Constructors
	/** Destroy this system. */
	inline ~PointSync_System() = default;
	/** Construct this system.
	@param	frameData	shared pointer of common data that changes frame-to-frame. */
	inline PointSync_System(const std::shared_ptr<PointData>& frameData)
		: m_frameData(frameData) {
		addComponentType(LightPoint_Component::m_ID, FLAG_REQUIRED);
		addComponentType(LightColor_Component::m_ID, FLAG_OPTIONAL);
		addComponentType(LightRadius_Component::m_ID, FLAG_OPTIONAL);
		addComponentType(Transform_Component::m_ID, FLAG_OPTIONAL);
		addComponentType(Shadow_Component::m_ID, FLAG_OPTIONAL);
		addComponentType(CameraArray_Component::m_ID, FLAG_OPTIONAL);
		addComponentType(BoundingBox_Component::m_ID, FLAG_OPTIONAL);
	}


	// Public Interface Implementations
	inline virtual void updateComponents(const float& deltaTime, const std::vector<std::vector<ecsBaseComponent*>>& components) override final {
		// Resize light buffers to match number of entities this frame
		m_frameData->lightBuffer.resize(components.size());
		m_frameData->lightBuffer.beginWriting();
		int index = 0;
		for each (const auto & componentParam in components) {
			auto* lightComponent = (LightPoint_Component*)componentParam[0];
			auto* colorComponent = (LightColor_Component*)componentParam[1];
			auto* radiusComponent = (LightRadius_Component*)componentParam[2];
			auto* transformComponent = (Transform_Component*)componentParam[3];
			auto* shadowComponent = (Shadow_Component*)componentParam[4];
			auto* cameraComponent = (CameraArray_Component*)componentParam[5];
			auto* bsphereComponent = (BoundingSphere_Component*)componentParam[6];

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

			// Sync Transform Attributes
			if (transformComponent) {
				const auto& position = transformComponent->m_worldTransform.m_position;
				m_frameData->lightBuffer[index].LightPosition = position;
				const glm::mat4 trans = glm::translate(glm::mat4(1.0f), position);
				const glm::mat4 scl = glm::scale(glm::mat4(1.0f), glm::vec3(radiusSquared * 1.1f));
				m_frameData->lightBuffer[index].mMatrix = (trans)*scl;
				const glm::mat4 pMatrix = glm::perspective(glm::radians(90.0f), 1.0f, 0.01f, radiusSquared);
				const glm::mat4 pMatrixInverse = glm::inverse(pMatrix);
				const glm::mat4 vMatrices[6] = {
					glm::lookAt(position, position + glm::vec3(1, 0, 0), glm::vec3(0, -1, 0)),
					glm::lookAt(position, position + glm::vec3(-1, 0, 0), glm::vec3(0, -1, 0)),
					glm::lookAt(position, position + glm::vec3(0, 1, 0), glm::vec3(0, 0, 1)),
					glm::lookAt(position, position + glm::vec3(0, -1, 0), glm::vec3(0, 0, -1)),
					glm::lookAt(position, position + glm::vec3(0, 0, 1), glm::vec3(0, -1, 0)),
					glm::lookAt(position, position + glm::vec3(0, 0, -1), glm::vec3(0, -1, 0))
				};
				if (cameraComponent) {
					cameraComponent->m_cameras.resize(6);
					cameraComponent->m_updateTimes.resize(6);
					for (int x = 0; x < 6; ++x) {
						auto& cam = cameraComponent->m_cameras[x];
						auto& camData = *cam.get();
						camData.Dimensions = glm::ivec2((int)m_frameData->shadowData->shadowSize);
						camData.FOV = 90.0f;
						camData.FarPlane = radiusSquared;
						camData.EyePosition = position;
						camData.pMatrix = pMatrix;
						camData.pMatrixInverse = pMatrixInverse;
						camData.vMatrix = vMatrices[x];
						camData.vMatrixInverse = glm::inverse(vMatrices[x]);
						camData.pvMatrix = pMatrix * vMatrices[x];
						cam.updateFrustum();
						m_frameData->lightBuffer[index].shadowVP[x] = pMatrix * vMatrices[x];
					}
				}
			}

			// Sync Buffer Attributes
			m_frameData->lightBuffer[index].Shadow_Spot = shadowComponent ? shadowComponent->m_shadowSpot : -1;
		}
		index++;
	}


private:
	// Private Attributes
	std::shared_ptr<PointData> m_frameData;
};

#endif // POINTSYNC_SYSTEM_H