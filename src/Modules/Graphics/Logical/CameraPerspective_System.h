#pragma once
#ifndef CAMERAPERSPECTIVE_SYSTEM_H
#define CAMERAPERSPECTIVE_SYSTEM_H

#include "Modules/ECS/ecsSystem.h"
#include "Modules/ECS/component_types.h"
#include "glm/gtc/type_ptr.hpp"
#include <memory>


/** An ECS system responsible for updating a shared pointer with a list of all active cameras in the scene. */
class CameraPerspective_System final : public ecsBaseSystem {
public:
	// Public (de)Constructors
	/** Destroy this system. */
	inline ~CameraPerspective_System() = default;
	/** Construct this system.
	@param	cameras		shared list of scene cameras. */
	inline CameraPerspective_System(const std::shared_ptr<std::vector<Camera*>> & cameras)
		: m_cameras(cameras) {
		addComponentType(Camera_Component::m_ID, FLAG_REQUIRED);
	}


	// Public Interface Implementations
	inline virtual void updateComponents(const float & deltaTime, const std::vector<std::vector<ecsBaseComponent*>> & components) override final {
		for each (const auto & componentParam in components) {
			Camera_Component * cameraComponent = (Camera_Component*)componentParam[0];
			m_cameras->push_back(&(cameraComponent->m_camera));
		}
	}


private:
	// Private Attributes
	std::shared_ptr<std::vector<Camera*>> m_cameras;
};

#endif // CAMERAPERSPECTIVE_SYSTEM_H