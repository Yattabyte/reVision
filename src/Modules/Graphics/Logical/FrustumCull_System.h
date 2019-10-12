#pragma once
#ifndef FRUSTUMCULL_SYSTEM_H
#define FRUSTUMCULL_SYSTEM_H

#include "Modules/ECS/ecsSystem.h"
#include "Modules/ECS/component_types.h"
#include "glm/gtc/type_ptr.hpp"
#include <memory>


/** An ECS system responsible for frustum culling all render-able components with a bounding sphere and a position. */
class FrustumCull_System final : public ecsBaseSystem {
public:
	// Public (de)Constructors
	/** Destroy this system. */
	inline ~FrustumCull_System() = default;
	/** Construct this system.
	@param	cameras		list of all the active cameras in the scene, updated per frame. */
	inline FrustumCull_System(const std::shared_ptr<std::vector<Camera*>>& sceneCameras)
		: m_sceneCameras(sceneCameras) {
		addComponentType(Transform_Component::m_ID, FLAG_REQUIRED);
		addComponentType(BoundingBox_Component::m_ID, FLAG_OPTIONAL);
		addComponentType(BoundingSphere_Component::m_ID, FLAG_OPTIONAL);
	}


	// Public Interface Implementations
	inline virtual void updateComponents(const float& deltaTime, const std::vector<std::vector<ecsBaseComponent*>>& components) override final {
		for each (const auto & componentParam in components) {
			auto* transformComponent = (Transform_Component*)componentParam[0];
			auto* bboxComponent = (BoundingBox_Component*)componentParam[1];
			auto* bsphereComponent = (BoundingSphere_Component*)componentParam[2];

			// Update the visibility status for each camera this entity is visible in
			for (int x = 0; x < m_sceneCameras->size(); ++x) {
				auto camera = m_sceneCameras->at(x);

				// Err on the side of caution and say its visible by default
				// Our visibility tests will try to EXCLUDE, not INCLUDE
				const auto& camPosition = camera->get()->EyePosition;
				auto objPosition = transformComponent->m_worldTransform.m_position;
				const auto objScale = transformComponent->m_worldTransform.m_scale;

				// If FOV is 360, it can see everything
				if (camera->get()->FOV < 359.9f) {
					if (bboxComponent) {
						objPosition += bboxComponent->m_positionOffset;
						// Treat it like a sphere
						const auto radius = glm::distance(bboxComponent->m_min * objScale, bboxComponent->m_max * objScale) / 2.0f;
						// Update bsphere with whether or not the camera is within it
						if (glm::distance(camPosition, objPosition) > radius)
							bboxComponent->m_cameraCollision = BoundingBox_Component::OUTSIDE;
						else
							bboxComponent->m_cameraCollision = BoundingBox_Component::INSIDE;
					}
					// Frustum x Bounding-Sphere Test
					if (bsphereComponent) {
						objPosition += bsphereComponent->m_positionOffset;
						const auto radius = bsphereComponent->m_radius;
						// Update bsphere with whether or not the camera is within it
						if (glm::distance(camPosition, objPosition) > radius)
							bsphereComponent->m_cameraCollision = BoundingSphere_Component::OUTSIDE;
						else
							bsphereComponent->m_cameraCollision = BoundingSphere_Component::INSIDE;
					}
				}
			}
		}
	}


private:
	// Private Attributes
	std::shared_ptr<std::vector<Camera*>> m_sceneCameras;
};

#endif // FRUSTUMCULL_SYSTEM_H