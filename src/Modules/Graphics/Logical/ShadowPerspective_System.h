#pragma once
#ifndef SHADOWPERSPECTIVE_SYSTEM_H
#define SHADOWPERSPECTIVE_SYSTEM_H

#include "Modules/ECS/ecsSystem.h"
#include "Modules/ECS/component_types.h"
#include <memory>


/** An ECS system responsible for collecting cameras from entities with shadow components. */
class ShadowPerspective_System final : public ecsBaseSystem {
public:
	// Public (De)Constructors
	/** Destroy this system. */
	inline ~ShadowPerspective_System() = default;
	/** Construct this system.
	@param	cameras		shared list of scene cameras. */
	inline explicit ShadowPerspective_System(const std::shared_ptr<std::vector<Camera*>>& sceneCameras)
		: m_sceneCameras(sceneCameras) {
		addComponentType(Shadow_Component::Runtime_ID, RequirementsFlag::FLAG_REQUIRED);
	}


	// Public Interface Implementations
	inline virtual void updateComponents(const float& deltaTime, const std::vector<std::vector<ecsBaseComponent*>>& components) noexcept override final {
		for each (const auto & componentParam in components) {
			auto* shadow = static_cast<Shadow_Component*>(componentParam[0]);
			for (auto& camera : shadow->m_cameras)
				m_sceneCameras->push_back(&camera);
		}
	}


private:
	// Private Attributes
	std::shared_ptr<std::vector<Camera*>> m_sceneCameras;
};

#endif // SHADOWPERSPECTIVE_SYSTEM_H