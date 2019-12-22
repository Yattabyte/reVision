#include "Modules/Graphics/Logical/FrustumCull_System.h"
#include "Modules/ECS/component_types.h"


FrustumCull_System::FrustumCull_System(std::vector<Camera*>& sceneCameras) :
	m_sceneCameras(sceneCameras)
{
	addComponentType(Transform_Component::Runtime_ID, RequirementsFlag::FLAG_REQUIRED);
	addComponentType(BoundingBox_Component::Runtime_ID, RequirementsFlag::FLAG_OPTIONAL);
	addComponentType(BoundingSphere_Component::Runtime_ID, RequirementsFlag::FLAG_OPTIONAL);
}

void FrustumCull_System::updateComponents(const float&, const std::vector<std::vector<ecsBaseComponent*>>& components) 
{
	for (const auto& componentParam : components) {
		const auto* transformComponent = static_cast<Transform_Component*>(componentParam[0]);
		auto* bboxComponent = static_cast<BoundingBox_Component*>(componentParam[1]);
		auto* bsphereComponent = static_cast<BoundingSphere_Component*>(componentParam[2]);

		// Update the visibility status for each camera this entity is visible in
		const auto sceneCameraCount = m_sceneCameras.size();
		for (int x = 0; x < sceneCameraCount; ++x) {
			const auto& camera = m_sceneCameras.at(x);

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
					// Update BSphere with whether or not the camera is within it
					if (glm::distance(camPosition, objPosition) > radius)
						bboxComponent->m_cameraCollision = BoundingBox_Component::CameraCollision::OUTSIDE;
					else
						bboxComponent->m_cameraCollision = BoundingBox_Component::CameraCollision::INSIDE;
				}
				// Frustum x Bounding-Sphere Test
				if (bsphereComponent) {
					objPosition += bsphereComponent->m_positionOffset;
					const auto radius = bsphereComponent->m_radius;
					// Update BSphere with whether or not the camera is within it
					if (glm::distance(camPosition, objPosition) > radius)
						bsphereComponent->m_cameraCollision = BoundingSphere_Component::CameraCollision::OUTSIDE;
					else
						bsphereComponent->m_cameraCollision = BoundingSphere_Component::CameraCollision::INSIDE;
				}
			}
		}
	}
}