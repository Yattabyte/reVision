#pragma once
#ifndef FRUSTUMCULL_SYSTEM_H
#define FRUSTUMCULL_SYSTEM_H

#include "Modules/World/ECS/ecsSystem.h"
#include "Modules/World/ECS/components.h"
#include "Modules/Graphics/Logical/components.h"
#include "glm/gtc/type_ptr.hpp"
#include <memory>


/** An ECS system responsible for frustum culling all renderable components with a bounding sphere and a position. */
class FrustumCull_System : public BaseECSSystem {
public:
	// Public (de)Constructors
	/** Destroy this system. */
	inline ~FrustumCull_System() = default;
	/** Construct this system. 
	@param	cameras		list of all the active cameras in the scene, updated per frame. */
	inline FrustumCull_System(const std::shared_ptr<std::vector<Camera*>> & cameras)
		: m_cameras(cameras) {
		addComponentType(Renderable_Component::ID, FLAG_REQUIRED);
		addComponentType(Transform_Component::ID, FLAG_REQUIRED);
		addComponentType(BoundingSphere_Component::ID, FLAG_OPTIONAL);
	}


	// Public Interface Implementations
	inline virtual void updateComponents(const float & deltaTime, const std::vector<std::vector<BaseECSComponent*>> & components) override {
		for each (const auto & componentParam in components) {
			Renderable_Component * renderableComponent = (Renderable_Component*)componentParam[0];
			Transform_Component * transformComponent = (Transform_Component*)componentParam[1];
			BoundingSphere_Component * bsphereComponent = (BoundingSphere_Component*)componentParam[2];

			// Ensure renderable component visibility and cameras are linked
			renderableComponent->m_visible.resize(m_cameras->size());

			// Update the visibility status for each camera this entity is visible in
			bool visibleAtAll = false;
			for (int x = 0; x < m_cameras->size(); ++x) {
				auto camera = m_cameras->at(x);

				// Err on the side of caution and say its visible by default
				// Our visibility tests will try to EXCLUDE, not INCLUDE
				bool isVisible = true;
				const auto & camPosition = camera->get()->EyePosition;
				auto objPosition = transformComponent->m_transform.m_position;
				if (bsphereComponent)
					objPosition += bsphereComponent->m_positionOffset;

				// If FOV is 360, it can see everything
				if (camera->get()->FOV < 359.9f) {
					// Frustum x Bounding-Sphere Test
					if (bsphereComponent) {
						const auto radius = bsphereComponent->m_radius;
						// Update bsphere with whether or not the camera is within it
						if (glm::distance(camPosition, objPosition) > radius)
							bsphereComponent->m_cameraCollision = BoundingSphere_Component::OUTSIDE;
						else
							bsphereComponent->m_cameraCollision = BoundingSphere_Component::INSIDE;

						for (int i = 0; i < 6; ++i) {
							if (camera->m_planes[i].x * objPosition.x + camera->m_planes[i].y * objPosition.y + camera->m_planes[i].z * objPosition.z + camera->m_planes[i].w <= -radius) {
								isVisible = false;
								break;
							}
						}
					}
				}
				else {
					// See if it falls outside our far plane
					// Consider the shape of the object
					if (bsphereComponent)
						objPosition -= bsphereComponent->m_radius;
					if (glm::distance(camPosition, objPosition) > camera->get()->FarPlane)
						isVisible = false;
				}

				if (isVisible)
					visibleAtAll = true;

				// Set visibility
				renderableComponent->m_visible[x] = isVisible;
			}

			renderableComponent->m_visibleAtAll = visibleAtAll;
		}
	}


private:
	// Private Attributes
	std::shared_ptr<std::vector<Camera*>> m_cameras;
};

#endif // FRUSTUMCULL_SYSTEM_H