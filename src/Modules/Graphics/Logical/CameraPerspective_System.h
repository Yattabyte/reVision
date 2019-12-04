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
	// Public (De)Constructors
	/** Destroy this system. */
	inline ~CameraPerspective_System() = default;
	/** Construct this system.
	@param	sceneCameras	reference to the scene cameras to use. */
	inline explicit CameraPerspective_System(std::vector<Camera*>& sceneCameras) noexcept :
		m_sceneCameras(sceneCameras)
	{
		addComponentType(Camera_Component::Runtime_ID, RequirementsFlag::FLAG_REQUIRED);
	}


	// Public Interface Implementations
	inline virtual void updateComponents(const float& deltaTime, const std::vector<std::vector<ecsBaseComponent*>>& components) noexcept override final {
		for (const auto& componentParam : components) {
			auto* cameraComponent = static_cast<Camera_Component*>(componentParam[0]);
			m_sceneCameras.push_back(&cameraComponent->m_camera);
		}
	}


private:
	// Private Attributes
	std::vector<Camera*>& m_sceneCameras;
};

#endif // CAMERAPERSPECTIVE_SYSTEM_H
