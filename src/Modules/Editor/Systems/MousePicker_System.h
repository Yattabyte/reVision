#pragma once
#ifndef MOUSEPICKER_SYSTEM_H
#define MOUSEPICKER_SYSTEM_H 

#include "Modules/World/ECS/ecsSystem.h"
#include "Modules/World/ECS/components.h"
#include "Utilities/Intersection.h"
#include "Engine.h"
#include "glm/glm.hpp"


/** An ECS system allowing the user to ray-pick entities by selecting against their components. */
class MousePicker_System : public BaseECSSystem {
public:
	// Public (de)Constructors
	/** Destroy this system. */
	inline ~MousePicker_System() {
		// Update indicator
		*m_aliveIndicator = false;
	}
	/** Construct this system.
	@param	engine		the currently active engine. */
	inline MousePicker_System(Engine * engine)
		: m_engine(engine) {
		// Declare component types used
		addComponentType(Transform_Component::ID);
		addComponentType(BoundingBox_Component::ID, FLAG_OPTIONAL);
		addComponentType(BoundingSphere_Component::ID, FLAG_OPTIONAL);
		addComponentType(Collider_Component::ID, FLAG_OPTIONAL);

		// Preferences
		auto & preferences = m_engine->getPreferenceState();
		preferences.getOrSetValue(PreferenceState::C_WINDOW_WIDTH, m_renderSize.x);
		preferences.getOrSetValue(PreferenceState::C_WINDOW_HEIGHT, m_renderSize.y);
		preferences.addCallback(PreferenceState::C_WINDOW_WIDTH, m_aliveIndicator, [&](const float &f) {
			m_renderSize.x = (int)f;
		});
		preferences.addCallback(PreferenceState::C_WINDOW_HEIGHT, m_aliveIndicator, [&](const float &f) {
			m_renderSize.y = (int)f;
		});
	}


	// Public Interface Implementation
	inline virtual void updateComponents(const float & deltaTime, const std::vector< std::vector<BaseECSComponent*> > & components) override {
		const auto & actionState = m_engine->getActionState();
		const auto & clientCamera = *m_engine->getModule_Graphics().getClientCamera()->get();
		const auto ray_origin = clientCamera.EyePosition;
		const auto ray_nds = glm::vec2(2.0f * actionState.at(ActionState::MOUSE_X) / m_renderSize.x - 1.0f, 1.0f - (2.0f * actionState.at(ActionState::MOUSE_Y)) / m_renderSize.y);
		const auto ray_eye = glm::vec4(glm::vec2(clientCamera.pMatrixInverse * glm::vec4(ray_nds, -1.0f, 1.0F)), -1.0f, 0.0f);
		const auto ray_world = glm::normalize(glm::vec3(clientCamera.vMatrixInverse * ray_eye));

		// Set the selection position for the worst-case scenario
		m_selectionTransform.m_position = clientCamera.EyePosition + (ray_world * glm::vec3(50.0f));

		float closestDistanceFromScreen = FLT_MAX, closestDistanceToCenter = FLT_MAX;
		for each (const auto & componentParam in components) {
			auto * transformComponent = (Transform_Component*)componentParam[0];
			auto * bBox = (BoundingBox_Component*)componentParam[1];
			auto * bSphere = (BoundingSphere_Component*)componentParam[2];
			auto * collider = (Collider_Component*)componentParam[3];

			bool checkSuccessfull = false;
			float distanceFromScreen = FLT_MAX, distanceToCenter = FLT_MAX;
			// Ray-collider intersection test
			if (collider) {
			}
			// Ray-OOBB intersection test
			if (bBox && bBox->m_cameraCollision == BoundingBox_Component::OUTSIDE) {
				float distance;
				const auto& position = transformComponent->m_worldTransform.m_position;
				const auto& scale = transformComponent->m_worldTransform.m_scale;
				const auto matrixWithoutScale = (transformComponent->m_worldTransform * Transform(bBox->m_positionOffset, glm::quat(1.0f, 0, 0, 0), 1.0f / scale)).m_modelMatrix;
				// Check if the distance is closer than the last entity found, so we can find the 'best' selection
				if (RayOOBBIntersection(ray_origin, ray_world, bBox->m_min * scale, bBox->m_max * scale, matrixWithoutScale, distance))
					distanceFromScreen = distance;
			}
			// Ray-BoundingSphere intersection test
			if (bSphere && bSphere->m_cameraCollision == BoundingSphere_Component::OUTSIDE) {
				// Check if the distance is closer than the last entity found, so we can find the 'best' selection
				if (auto distance = RaySphereIntersection(ray_origin, ray_world, transformComponent->m_worldTransform.m_position + bSphere->m_positionOffset, bSphere->m_radius); distance >= 0.0f)
					distanceFromScreen = distance;
			}
			// Ray-Origin intersection test
			{
				// Create scaling factor to keep all origins same screen size
				const auto radius = glm::distance(transformComponent->m_worldTransform.m_position, m_engine->getModule_Graphics().getClientCamera()->get()->EyePosition) * 0.033f;

				// Check if the distance is closer than the last entity found, so we can find the 'best' selection
				if (auto distance = RaySphereIntersection(ray_origin, ray_world, transformComponent->m_worldTransform.m_position, radius); distance >= 0.0f)
					distanceFromScreen = distance;
			}

			auto hitToCam = clientCamera.pMatrix * clientCamera.vMatrix * glm::vec4(ray_origin + distanceToCenter * ray_world, 1.0f); 
			hitToCam /= hitToCam.w;
			const auto hit_ss = (0.5f * glm::vec2(hitToCam) + 0.5f) * glm::vec2(m_renderSize);
			auto centerToCam = clientCamera.pMatrix * clientCamera.vMatrix * glm::vec4(transformComponent->m_worldTransform.m_position, 1.0f);
			centerToCam /= centerToCam.w;
			const auto center_ss = (0.5f * glm::vec2(centerToCam) + 0.5f) * glm::vec2(m_renderSize);
			distanceToCenter = glm::length(center_ss - hit_ss);

			// Find the closest best match
			if (distanceFromScreen < closestDistanceFromScreen || distanceToCenter < closestDistanceToCenter) {
				closestDistanceFromScreen = distanceFromScreen;
				closestDistanceToCenter = distanceToCenter;
				m_selection = transformComponent->entity->m_uuid;
				m_selectionTransform = transformComponent->m_worldTransform;
			}
		}
	}  


	// Public Methods
	/** Retrieve this system's last selection result. */
	std::pair<ecsHandle, Transform> getSelection() {
		return { m_selection, m_selectionTransform };
	}


private:
	// Private Attributes
	Engine * m_engine = nullptr;
	ecsHandle m_selection;
	Transform m_selectionTransform = glm::vec3(0.0f);
	glm::ivec2 m_renderSize = glm::ivec2(1);
	std::shared_ptr<bool> m_aliveIndicator = std::make_shared<bool>(true);
};

#endif // MOUSEPICKER_SYSTEM_H