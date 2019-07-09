#pragma once
#ifndef REFLECTORSYNC_SYSTEM_H
#define REFLECTORSYNC_SYSTEM_H

#include "Modules/World/ECS/ecsSystem.h"
#include "Modules/World/ECS/components.h"
#include "Modules/Graphics/Logical/components.h"
#include "Modules/Graphics/Lighting/components.h"
#include "Modules/Graphics/Lighting/Reflector/ReflectorData.h"


/***/
class ReflectorSync_System : public BaseECSSystem {
public:
	// Public (de)Constructors
	/***/
	inline ~ReflectorSync_System() = default;
	/***/
	inline ReflectorSync_System(const std::shared_ptr<ReflectorData> & frameData)
		: m_frameData(frameData) {
		addComponentType(Renderable_Component::ID, FLAG_REQUIRED);
		addComponentType(Reflector_Component::ID, FLAG_REQUIRED);
		addComponentType(Transform_Component::ID, FLAG_REQUIRED);
		addComponentType(CameraArray_Component::ID, FLAG_REQUIRED);
	}


	// Public Interface Implementations
	inline virtual void updateComponents(const float & deltaTime, const std::vector<std::vector<BaseECSComponent*>> & components) override {
		// Resize light buffers to match number of entities this frame
		m_frameData->lightBuffer.resize(components.size());
		m_frameData->lightBuffer.beginWriting();
		int index = 0;
		for each (const auto & componentParam in components) {
			Renderable_Component * renderableComponent = (Renderable_Component*)componentParam[0];
			Reflector_Component * reflectorComponent = (Reflector_Component*)componentParam[1];
			Transform_Component * transformComponent = (Transform_Component*)componentParam[2];
			CameraArray_Component * cameraComponent = (CameraArray_Component*)componentParam[3];

			// Synchronize the component if it is visible
			if (renderableComponent->m_visibleAtAll) {
				const auto & position = transformComponent->m_transform.m_position;
				const auto & orientation = transformComponent->m_transform.m_orientation;
				const auto & scale = transformComponent->m_transform.m_scale;
				const auto & modelMatrix = transformComponent->m_transform.m_modelMatrix;
				const auto matRot = glm::mat4_cast(orientation);
				const float largest = pow(std::max(std::max(scale.x, scale.y), scale.z), 2.0f);
				m_frameData->lightBuffer[index].mMatrix = modelMatrix;
				m_frameData->lightBuffer[index].rotMatrix = glm::inverse(matRot);
				m_frameData->lightBuffer[index].BoxCamPos = position;
				m_frameData->lightBuffer[index].BoxScale = scale;
				const glm::mat4 pMatrix = glm::perspective(glm::radians(90.0f), 1.0f, 0.01f, largest);
				const glm::mat4 pMatrixInverse = glm::inverse(pMatrix);
				const glm::mat4 vMatrices[6] = {
					glm::lookAt(position, position + glm::vec3(1, 0, 0), glm::vec3(0, -1, 0)),
					glm::lookAt(position, position + glm::vec3(-1, 0, 0), glm::vec3(0, -1, 0)),
					glm::lookAt(position, position + glm::vec3(0, 1, 0), glm::vec3(0, 0, 1)),
					glm::lookAt(position, position + glm::vec3(0, -1, 0), glm::vec3(0, 0, -1)),
					glm::lookAt(position, position + glm::vec3(0, 0, 1), glm::vec3(0, -1, 0)),
					glm::lookAt(position, position + glm::vec3(0, 0, -1), glm::vec3(0, -1, 0))
				};
				cameraComponent->m_cameras.resize(6);
				for (int x = 0; x < 6; ++x) {
					auto & cam = cameraComponent->m_cameras[x];
					cam.Dimensions = m_frameData->envmapSize;
					cam.FOV = 90.0f;
					cam.FarPlane = largest;
					cam.EyePosition = position;
					cam.pMatrix = pMatrix;
					cam.pMatrixInverse = pMatrixInverse;
					cam.vMatrix = vMatrices[x];
					cam.vMatrixInverse = glm::inverse(vMatrices[x]);
					cam.pvMatrix = pMatrix * vMatrices[x];
				}

				// Sync Buffer Attributes
				m_frameData->lightBuffer[index].CubeSpot = reflectorComponent->m_cubeSpot;
			}
			index++;
		}
	}


private:
	// Private Attributes
	std::shared_ptr<ReflectorData> m_frameData;
};

#endif // REFLECTORSYNC_SYSTEM_H