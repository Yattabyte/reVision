#pragma once
#ifndef POINTSYNC_SYSTEM_H
#define POINTSYNC_SYSTEM_H

#include "Modules/World/ECS/ecsSystem.h"
#include "Modules/World/ECS/components.h"
#include "Modules/Graphics/Logical/components.h"
#include "Modules/Graphics/Lighting/components.h"
#include "Modules/Graphics/Lighting/Point/PointData.h"


/***/
class PointSync_System : public BaseECSSystem {
public:
	// Public (de)Constructors
	/***/
	inline ~PointSync_System() = default;
	/***/
	inline PointSync_System(const std::shared_ptr<PointData> & frameData)
		: m_frameData(frameData) {
		addComponentType(Renderable_Component::ID, FLAG_REQUIRED);
		addComponentType(LightPoint_Component::ID, FLAG_REQUIRED);
		addComponentType(LightColor_Component::ID, FLAG_OPTIONAL);
		addComponentType(LightRadius_Component::ID, FLAG_OPTIONAL);
		addComponentType(Transform_Component::ID, FLAG_OPTIONAL);
		addComponentType(CameraArray_Component::ID, FLAG_OPTIONAL);
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
			LightPoint_Component * lightComponent = (LightPoint_Component*)componentParam[1];
			LightColor_Component * colorComponent = (LightColor_Component*)componentParam[2];
			LightRadius_Component * radiusComponent = (LightRadius_Component*)componentParam[3];
			Transform_Component * transformComponent = (Transform_Component*)componentParam[4];
			CameraArray_Component * cameraComponent = (CameraArray_Component*)componentParam[5];
			BoundingSphere_Component * bsphereComponent = (BoundingSphere_Component*)componentParam[6];

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

				// Sync Transform Attributes
				if (transformComponent) {
					const auto & position = transformComponent->m_transform.m_position;
					m_frameData->lightBuffer[index].LightPosition = position;
					const glm::mat4 trans = glm::translate(glm::mat4(1.0f), position);
					const glm::mat4 scl = glm::scale(glm::mat4(1.0f), glm::vec3(radiusSquared *1.1f));
					m_frameData->lightBuffer[index].mMatrix = (trans)* scl;
					const glm::mat4 pMatrix = glm::perspective(glm::radians(90.0f), 1.0f, 0.01f, radiusSquared);					
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
						for (int x = 0; x < 6; ++x) {
							if (!cameraComponent->m_cameras[x])
								cameraComponent->m_cameras[x] = std::make_shared<CameraBuffer>();
							cameraComponent->m_cameras[x]->get()->Dimensions = m_frameData->shadowSize;
							cameraComponent->m_cameras[x]->get()->FOV = 90.0f;
							cameraComponent->m_cameras[x]->get()->FarPlane = radiusSquared;
							cameraComponent->m_cameras[x]->get()->EyePosition = position;
							cameraComponent->m_cameras[x]->get()->pMatrix = pMatrix;
							cameraComponent->m_cameras[x]->get()->vMatrix = vMatrices[x];
						}
					}
				}

				// Sync Buffer Attributes
				m_frameData->lightBuffer[index].Shadow_Spot = lightComponent->m_shadowSpot;
			}
			index++;
		}
	}


private:
	// Private Attributes
	std::shared_ptr<PointData> m_frameData;
};

#endif // POINTSYNC_SYSTEM_H