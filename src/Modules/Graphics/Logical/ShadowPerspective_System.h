#pragma once
#ifndef SHADOWPERSPECTIVE_SYSTEM_H
#define SHADOWPERSPECTIVE_SYSTEM_H

#include "Modules/ECS/ecsSystem.h"
#include "Modules/ECS/component_types.h"
#include <memory>


/***/
class ShadowPerspective_System final : public ecsBaseSystem {
public:
	// Public (de)Constructors
	/** Destroy this system. */
	inline ~ShadowPerspective_System() = default;
	/** Construct this system.
	@param	cameras		shared list of scene cameras. */
	inline ShadowPerspective_System(const std::shared_ptr<std::vector<Camera*>>& sceneCameras)
		: m_sceneCameras(sceneCameras) {
		addComponentType(Shadow_Component::m_ID, FLAG_REQUIRED);
	}


	// Public Interface Implementations
	inline virtual void updateComponents(const float& deltaTime, const std::vector<std::vector<ecsBaseComponent*>>& components) override final {
		for each (const auto & componentParam in components) {
			auto* shadow = (Shadow_Component*)componentParam[0];
			for (auto& camera : shadow->m_cameras)
				m_sceneCameras->push_back(&camera);
		}
	}


private:
	// Private Attributes
	std::shared_ptr<std::vector<Camera*>> m_sceneCameras;
};

#endif // SHADOWPERSPECTIVE_SYSTEM_H