#pragma once
#ifndef SPOTSYNC_SYSTEM_H
#define SPOTSYNC_SYSTEM_H

#include "Modules/World/ECS/ecsSystem.h"
#include "Modules/World/ECS/components.h"
#include "Modules/Graphics/Logical/components.h"
#include "Modules/Graphics/Lighting/components.h"
#include "Modules/Graphics/Lighting/Point/PointData.h"


/***/
class SpotSync_System : public BaseECSSystem {
public:
	// Public (de)Constructors
	/***/
	inline ~SpotSync_System() = default;
	/***/
	inline SpotSync_System(const std::shared_ptr<SpotData> & frameData)
		: m_frameData(frameData) {
		addComponentType(Renderable_Component::ID, FLAG_REQUIRED);
		addComponentType(LightSpot_Component::ID, FLAG_REQUIRED);
		addComponentType(LightColor_Component::ID, FLAG_OPTIONAL);
		addComponentType(LightRadius_Component::ID, FLAG_OPTIONAL);
		addComponentType(LightCutoff_Component::ID, FLAG_OPTIONAL);
		addComponentType(Transform_Component::ID, FLAG_OPTIONAL);
		addComponentType(Shadow_Component::ID, FLAG_OPTIONAL);
		addComponentType(Camera_Component::ID, FLAG_OPTIONAL);
		addComponentType(BoundingSphere_Component::ID, FLAG_OPTIONAL);
	}


	// Public Interface Implementations
	inline virtual void updateComponents(const float & deltaTime, const std::vector<std::vector<BaseECSComponent*>> & components) override {
		// Resize light buffers to match number of entities this frame
		m_frameData->lightBuffer.resize(components.size());
		m_frameData->lightBuffer.beginWriting();
		int index = 0;
		for each (const auto & componentParam in components) {
			Renderable_Component * renderableComponent = (Renderable_Component*)componentParam[0];
			LightSpot_Component * lightComponent = (LightSpot_Component*)componentParam[1];
			LightColor_Component * colorComponent = (LightColor_Component*)componentParam[2];
			LightRadius_Component * radiusComponent = (LightRadius_Component*)componentParam[3];
			LightCutoff_Component * cutoffComponent = (LightCutoff_Component*)componentParam[4];
			Transform_Component * transformComponent = (Transform_Component*)componentParam[5];
			Shadow_Component * shadowComponent = (Shadow_Component*)componentParam[6];
			Camera_Component * cameraComponent = (Camera_Component*)componentParam[7];
			BoundingSphere_Component * bsphereComponent = (BoundingSphere_Component*)componentParam[8];


			// Synchronize the component if it is visible
			if (renderableComponent->m_visibleAtAll) {
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
					const auto & position = transformComponent->m_transform.m_position;
					const auto & orientation = transformComponent->m_transform.m_orientation;
					const auto matRot = glm::mat4_cast(orientation);
					m_frameData->lightBuffer[index].LightPosition = position;
					auto dir = matRot * glm::vec4(0, 0, -1, 1);
					dir /= dir.w;
					m_frameData->lightBuffer[index].LightDirection = glm::normalize(glm::vec3(dir));
					const glm::mat4 trans = glm::translate(glm::mat4(1.0f), position);
					const glm::mat4 scl = glm::scale(glm::mat4(1.0f), glm::vec3(radiusSquared)*1.1f);
					m_frameData->lightBuffer[index].mMatrix = trans * matRot * scl;

					const glm::mat4 pMatrix = glm::perspective(glm::radians(cutoff * 2.0f), 1.0f, 0.01f, radiusSquared);
					const glm::mat4 pMatrixInverse = glm::inverse(pMatrix);
					const auto pv = pMatrix * glm::inverse(trans * glm::mat4_cast(orientation));
					m_frameData->lightBuffer[index].lightPV = pv;
					if (cameraComponent) {
						auto & cam = cameraComponent->m_camera;
						auto & camData = *cam.get();
						camData.Dimensions = glm::ivec2(m_frameData->shadowData->shadowSize);
						camData.FarPlane = radiusSquared;
						camData.EyePosition = position;
						camData.FOV = cutoff * 2.0f;
						camData.pMatrix = pMatrix;
						camData.pMatrixInverse = pMatrixInverse;
						camData.vMatrix = glm::inverse(trans * glm::mat4_cast(orientation));
						camData.vMatrixInverse = (trans * glm::mat4_cast(orientation)); // yes, this is correct
						camData.pvMatrix = pMatrix * camData.vMatrix;
						cam.updateFrustum();
					}
				}

				// Sync Buffer Attributes
				m_frameData->lightBuffer[index].Shadow_Spot = shadowComponent ? shadowComponent->m_shadowSpot : -1;
			}
			index++;
		}
	}


private:
	// Private Attributes
	std::shared_ptr<SpotData> m_frameData;
};

#endif // SPOTSYNC_SYSTEM_H