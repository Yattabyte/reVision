#pragma once
#ifndef REFLECTORSYNC_SYSTEM_H
#define REFLECTORSYNC_SYSTEM_H

#include "Modules/World/ECS/ecsSystem.h"
#include "Modules/World/ECS/components.h"
#include "Modules/Graphics/Logical/components.h"
#include "Modules/Graphics/Lighting/components.h"
#include "Utilities/GL/GL_ArrayBuffer.h"
#include <memory>


/***/
struct Reflector_Buffers {
	/** OpenGL buffer for Parallax reflectors. */
	struct Reflector_Buffer {
		glm::mat4 mMatrix;
		glm::mat4 rotMatrix;
		glm::vec3 BoxCamPos; float padding1;
		glm::vec3 BoxScale; float padding2;
		int CubeSpot; glm::vec3 padding3;
	};
	GL_ArrayBuffer<Reflector_Buffer> reflectorBuffer;
	bool envmapOutOfDate = false;
	float envmapSize = 1.0f;
};

/***/
class ReflectorSync_System : public BaseECSSystem {
public:
	// Public (de)Constructors
	/***/
	inline ~ReflectorSync_System() = default;
	/***/
	inline ReflectorSync_System(const std::shared_ptr<Reflector_Buffers> & buffers)
		: m_buffers(buffers) {
		addComponentType(Renderable_Component::ID, FLAG_REQUIRED);
		addComponentType(Reflector_Component::ID, FLAG_REQUIRED);
		addComponentType(Transform_Component::ID, FLAG_REQUIRED);
		addComponentType(Viewport_Component::ID, FLAG_REQUIRED);
	}


	// Public Interface Implementations
	inline virtual void updateComponents(const float & deltaTime, const std::vector<std::vector<BaseECSComponent*>> & components) override {
		for each (const auto & componentParam in components) {
			Renderable_Component * renderableComponent = (Renderable_Component*)componentParam[0];
			Reflector_Component * reflectorComponent = (Reflector_Component*)componentParam[1];
			Transform_Component * transformComponent = (Transform_Component*)componentParam[2];
			Viewport_Component * viewportComponent = (Viewport_Component*)componentParam[3];
			const auto & index = reflectorComponent->m_reflectorIndex;

			// Relay when shadows need to be rebuilt
			if (m_buffers->envmapOutOfDate)
				reflectorComponent->m_sceneOutOfDate = true;

			// Synchronize the component if it is visible
			if (renderableComponent->m_visibleAtAll) {
				const auto & position = transformComponent->m_transform.m_position;
				const auto & orientation = transformComponent->m_transform.m_orientation;
				const auto & scale = transformComponent->m_transform.m_scale;
				const auto & modelMatrix = transformComponent->m_transform.m_modelMatrix;
				const auto matRot = glm::mat4_cast(orientation);
				const float largest = pow(std::max(std::max(scale.x, scale.y), scale.z), 2.0f);
				m_buffers->reflectorBuffer[index].mMatrix = modelMatrix;
				m_buffers->reflectorBuffer[index].rotMatrix = glm::inverse(matRot);
				m_buffers->reflectorBuffer[index].BoxCamPos = position;
				m_buffers->reflectorBuffer[index].BoxScale = scale;
				const glm::mat4 vMatrices[6] = {
					glm::lookAt(position, position + glm::vec3(1, 0, 0), glm::vec3(0, -1, 0)),
					glm::lookAt(position, position + glm::vec3(-1, 0, 0), glm::vec3(0, -1, 0)),
					glm::lookAt(position, position + glm::vec3(0, 1, 0), glm::vec3(0, 0, 1)),
					glm::lookAt(position, position + glm::vec3(0, -1, 0), glm::vec3(0, 0, -1)),
					glm::lookAt(position, position + glm::vec3(0, 0, 1), glm::vec3(0, -1, 0)),
					glm::lookAt(position, position + glm::vec3(0, 0, -1), glm::vec3(0, -1, 0))
				};
				const glm::mat4 pMatrix = glm::perspective(glm::radians(90.0f), 1.0f, 0.01f, largest);
				for (int x = 0; x < 6; ++x) {					
					reflectorComponent->m_cameras[x].Dimensions = glm::vec2(m_buffers->envmapSize);
					reflectorComponent->m_cameras[x].FOV = 90.0f;
					reflectorComponent->m_cameras[x].FarPlane = largest;
					reflectorComponent->m_cameras[x].EyePosition = position;
					reflectorComponent->m_cameras[x].pMatrix = pMatrix;
					reflectorComponent->m_cameras[x].vMatrix = vMatrices[x];
				}

				viewportComponent->m_camera->resize(glm::vec2(m_buffers->envmapSize));

				// Sync Buffer Attributes
				m_buffers->reflectorBuffer[index].CubeSpot = reflectorComponent->m_cubeSpot;
			}
		}
	}

private:
	// Private Attributes
	std::shared_ptr<Reflector_Buffers> m_buffers;
};

#endif // REFLECTORSYNC_SYSTEM_H