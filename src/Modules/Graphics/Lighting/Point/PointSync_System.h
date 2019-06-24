#pragma once
#ifndef POINTSYNC_SYSTEM_H
#define POINTSYNC_SYSTEM_H

#include "Modules/World/ECS/ecsSystem.h"
#include "Modules/World/ECS/components.h"
#include "Modules/Graphics/Logical/components.h"
#include "Modules/Graphics/Lighting/components.h"
#include "Utilities/GL/GL_ArrayBuffer.h"
#include <memory>


/***/
struct Point_Buffers {
	/** OpenGL buffer for point lights. */
	struct Point_Buffer {
		glm::mat4 lightV;
		glm::mat4 lightPV[6];
		glm::mat4 inversePV[6];
		glm::mat4 mMatrix;
		glm::vec3 LightColor; float padding1;
		glm::vec3 LightPosition; float padding2;
		float LightIntensity;
		float LightRadius;
		int Shadow_Spot; float padding3;
	};
	GL_ArrayBuffer<Point_Buffer> lightBuffer;
	bool shadowOutOfDate = false;
};

/***/
class PointSync_System : public BaseECSSystem {
public:
	// Public (de)Constructors
	/***/
	inline ~PointSync_System() = default;
	/***/
	inline PointSync_System(const std::shared_ptr<Point_Buffers> & buffers)
		: m_buffers(buffers) {
		addComponentType(Renderable_Component::ID, FLAG_REQUIRED);
		addComponentType(LightPoint_Component::ID, FLAG_REQUIRED);
		addComponentType(LightColor_Component::ID, FLAG_OPTIONAL);
		addComponentType(LightRadius_Component::ID, FLAG_OPTIONAL);
		addComponentType(Transform_Component::ID, FLAG_OPTIONAL);
		addComponentType(BoundingSphere_Component::ID, FLAG_OPTIONAL);
	}


	// Public Interface Implementations
	inline virtual void updateComponents(const float & deltaTime, const std::vector<std::vector<BaseECSComponent*>> & components) override {
		for each (const auto & componentParam in components) {
			Renderable_Component * renderableComponent = (Renderable_Component*)componentParam[0];
			LightPoint_Component * lightComponent = (LightPoint_Component*)componentParam[1];
			LightColor_Component * colorComponent = (LightColor_Component*)componentParam[2];
			LightRadius_Component * radiusComponent = (LightRadius_Component*)componentParam[3];
			Transform_Component * transformComponent = (Transform_Component*)componentParam[4];
			BoundingSphere_Component * bsphereComponent = (BoundingSphere_Component*)componentParam[5];
			const auto & index = lightComponent->m_lightIndex;

			// Relay when shadows need to be rebuilt
			if (m_buffers->shadowOutOfDate)
				lightComponent->m_staticOutOfDate = true;			

			// Synchronize the component if it is visible
			if (renderableComponent->m_visible) {
				// Sync Color Attributes
				if (colorComponent) {
					m_buffers->lightBuffer[index].LightColor = colorComponent->m_color;
					m_buffers->lightBuffer[index].LightIntensity = colorComponent->m_intensity;
				}

				// Sync Radius Attributes
				float radiusSquared = 1.0f;
				if (radiusComponent) {
					m_buffers->lightBuffer[index].LightRadius = radiusComponent->m_radius;
					radiusSquared = radiusComponent->m_radius * radiusComponent->m_radius;
				}
				if (bsphereComponent)
					bsphereComponent->m_radius = radiusSquared;

				// Sync Transform Attributes
				if (transformComponent) {
					const auto & position = transformComponent->m_transform.m_position;
					lightComponent->m_position = position;
					m_buffers->lightBuffer[index].LightPosition = position;
					const glm::mat4 trans = glm::translate(glm::mat4(1.0f), position);
					const glm::mat4 scl = glm::scale(glm::mat4(1.0f), glm::vec3(radiusSquared *1.1f));
					m_buffers->lightBuffer[index].mMatrix = (trans)* scl;

					if (lightComponent->m_hasShadow) {
						m_buffers->lightBuffer[index].lightV = glm::translate(glm::mat4(1.0f), -position);
						glm::mat4 rotMats[6];
						const glm::mat4 pMatrix = glm::perspective(glm::radians(90.0f), 1.0f, 0.01f, radiusSquared);
						rotMats[0] = pMatrix * glm::lookAt(position, position + glm::vec3(1, 0, 0), glm::vec3(0, -1, 0));
						rotMats[1] = pMatrix * glm::lookAt(position, position + glm::vec3(-1, 0, 0), glm::vec3(0, -1, 0));
						rotMats[2] = pMatrix * glm::lookAt(position, position + glm::vec3(0, 1, 0), glm::vec3(0, 0, 1));
						rotMats[3] = pMatrix * glm::lookAt(position, position + glm::vec3(0, -1, 0), glm::vec3(0, 0, -1));
						rotMats[4] = pMatrix * glm::lookAt(position, position + glm::vec3(0, 0, 1), glm::vec3(0, -1, 0));
						rotMats[5] = pMatrix * glm::lookAt(position, position + glm::vec3(0, 0, -1), glm::vec3(0, -1, 0));
						for (int x = 0; x < 6; ++x) {
							m_buffers->lightBuffer[index].lightPV[x] = rotMats[x];
							m_buffers->lightBuffer[index].inversePV[x] = glm::inverse(rotMats[x]);
						}
					}
				}

				// Sync Buffer Attributes
				m_buffers->lightBuffer[index].Shadow_Spot = lightComponent->m_shadowSpot;
			}
		}
	}

private:
	// Private Attributes
	std::shared_ptr<Point_Buffers> m_buffers;
};

#endif // POINTSYNC_SYSTEM_H