#pragma once
#ifndef MOUSEPICKER_SYSTEM_H
#define MOUSEPICKER_SYSTEM_H 

#include "Modules/World/ECS/ecsSystem.h"
#include "Modules/World/ECS/components.h"
#include "Engine.h"
#include "glm/glm.hpp"


/***/
class MousePicker_System : public BaseECSSystem {
public:
	// Public (de)Constructors
	/***/
	inline ~MousePicker_System() {
		// Update indicator
		*m_aliveIndicator = false;
	}
	/***/
	inline MousePicker_System(Engine * engine)
		: m_engine(engine) {
		// Declare component types used
		addComponentType(Transform_Component::ID);
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
		m_selectionPosition = clientCamera.EyePosition + (ray_world * glm::vec3(50.0f));

		const auto hit_sphere = [&](const glm::vec3 & center, const float & radius) {
			const auto oc = ray_origin - center;
			const auto a = glm::dot(ray_world, ray_world);
			const auto b = 2.0f * glm::dot(oc, ray_world);
			const auto c = glm::dot(oc, oc) - radius * radius;
			const auto discriminant = b * b - 4.0f * a * c;
			if (discriminant < 0.0f)
				return -1.0f;
			else {
				float numerator = -b - glm::sqrt(discriminant);
				if (numerator > 0.0)
					return numerator / (2.0f * a);
				numerator = -b + glm::sqrt(discriminant);
				if (numerator > 0.0)
					return numerator / (2.0f * a);
				else
					return -1.0f;
			}
		};

		std::tuple<ecsEntity*, float, glm::vec3> closest = { NULL_ENTITY_HANDLE, FLT_MAX, glm::vec3(0.0f) };
		for each (const auto & componentParam in components) {
			auto * transformComponent = (Transform_Component*)componentParam[0];
			auto * bSphere = (BoundingSphere_Component*)componentParam[1];
			auto * collider = (Collider_Component*)componentParam[2];

			bool checkSuccessfull = false;
			if (collider) {
				// Ray-Collider intersection test
			}
			else if (bSphere && !checkSuccessfull) {
				// Check if the distance is closer than the last entity found, so we can find the 'best' selection
				if (auto distance = hit_sphere(transformComponent->m_worldTransform.m_position + bSphere->m_positionOffset, bSphere->m_radius); distance >= 0.0f)
					if (distance < std::get<1>(closest)) {
						checkSuccessfull = true;
						closest = { transformComponent->entity, distance, transformComponent->m_worldTransform.m_position };
					}

			}
			else if (!checkSuccessfull) {
				// Create scaling factor to keep all origins same screen size
				const auto radius = glm::distance(transformComponent->m_worldTransform.m_position, m_engine->getModule_Graphics().getClientCamera()->get()->EyePosition) * 0.033f;

				// Check if the distance is closer than the last entity found, so we can find the 'best' selection
				if (auto distance = hit_sphere(transformComponent->m_worldTransform.m_position, radius); distance >= 0.0f)
					if (distance < std::get<1>(closest)) {
						checkSuccessfull = true;
						closest = { transformComponent->entity, distance, transformComponent->m_worldTransform.m_position };
					}
			}

			// Ensure we have a valid entity selected
			if (const auto &[entity, distance, position] = closest; entity != NULL_ENTITY_HANDLE) {
				m_selection = entity;
				m_selectionPosition = position;
			}
		}
	}


	// Public Methods
	/***/
	std::pair<ecsEntity*, glm::vec3> getSelection() {
		return { m_selection, m_selectionPosition };
	}


private:
	// Private Attributes
	Engine * m_engine = nullptr;
	ecsEntity * m_selection = NULL_ENTITY_HANDLE;
	glm::vec3 m_selectionPosition = glm::vec3(0.0f);
	glm::ivec2 m_renderSize = glm::ivec2(1);
	std::shared_ptr<bool> m_aliveIndicator = std::make_shared<bool>(true);
};

#endif // MOUSEPICKER_SYSTEM_H