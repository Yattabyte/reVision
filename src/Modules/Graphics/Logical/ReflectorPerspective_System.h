#pragma once
#ifndef REFLECTORPERSPECTIVE_SYSTEM_H
#define REFLECTORPERSPECTIVE_SYSTEM_H

#include "Modules/ECS/ecsSystem.h"
#include "Modules/ECS/component_types.h"
#include "glm/gtc/type_ptr.hpp"
#include <memory>


/** An ECS system responsible for updating a shared pointer with a list of all active cameras from reflectors in the scene. */
class ReflectorPerspective_System final : public ecsBaseSystem {
public:
	// Public (De)Constructors
	/** Destroy this system. */
	inline ~ReflectorPerspective_System() = default;
	/** Construct this system.
	@param	cameras		shared list of scene cameras. */
	inline explicit ReflectorPerspective_System(const std::shared_ptr<std::vector<Camera*>>& sceneCameras)
		: m_sceneCameras(sceneCameras) {
		addComponentType(Reflector_Component::Runtime_ID, RequirementsFlag::FLAG_REQUIRED);
	}


	// Public Interface Implementations
	inline virtual void updateComponents(const float& deltaTime, const std::vector<std::vector<ecsBaseComponent*>>& components) override final {
		for each (const auto & componentParam in components) {
			auto* cameraComponent = static_cast<Reflector_Component*>(componentParam[0]);
			for (auto& camera : cameraComponent->m_cameras)
				m_sceneCameras->push_back(&camera);
		}
	}


private:
	// Private Attributes
	std::shared_ptr<std::vector<Camera*>> m_sceneCameras;
};

#endif // REFLECTORPERSPECTIVE_SYSTEM_H