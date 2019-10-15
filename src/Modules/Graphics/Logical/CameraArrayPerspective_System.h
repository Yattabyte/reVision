#pragma once
#ifndef CAMERAARRAYPERSPECTIVE_SYSTEM_H
#define CAMERAARRAYPERSPECTIVE_SYSTEM_H

#include "Modules/ECS/ecsSystem.h"
#include "Modules/ECS/component_types.h"
#include "glm/gtc/type_ptr.hpp"
#include <memory>


/** An ECS system responsible for updating a shared pointer with a list of all active cameras in the scene. */
class CameraArrayPerspective_System final : public ecsBaseSystem {
public:
	// Public (de)Constructors
	/** Destroy this system. */
	inline ~CameraArrayPerspective_System() = default;
	/** Construct this system.
	@param	cameras		shared list of scene cameras. */
	inline CameraArrayPerspective_System(const std::shared_ptr<std::vector<Camera*>>& sceneCameras)
		: m_sceneCameras(sceneCameras) {
		addComponentType(CameraArray_Component::m_ID, FLAG_REQUIRED);
	}


	// Public Interface Implementations
	inline virtual void updateComponents(const float& deltaTime, const std::vector<std::vector<ecsBaseComponent*>>& components) override final {
		for each (const auto & componentParam in components) {
			auto* cameraComponent = (CameraArray_Component*)componentParam[0];
			for (auto& camera : cameraComponent->m_cameras)
				m_sceneCameras->push_back(&camera);
		}
	}


private:
	// Private Attributes
	std::shared_ptr<std::vector<Camera*>> m_sceneCameras;
};

#endif // CAMERAARRAYPERSPECTIVE_SYSTEM_H