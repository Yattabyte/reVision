#pragma once
#ifndef TRANSFORM_SYSTEM_H
#define TRANSFORM_SYSTEM_H

#include "Modules/World/ECS/ecsSystem.h"
#include "Modules/World/ECS/components.h"
#include "glm/gtc/type_ptr.hpp"
#include <memory>


/***/
class Transform_System : public BaseECSSystem {
public:
	// Public (de)Constructors
	/** Destroy this system. */
	inline ~Transform_System() = default;
	/** Construct this system.
	@param	cameras		shared list of scene cameras. */
	inline Transform_System() {
		addComponentType(Transform_Component::ID, FLAG_REQUIRED);
	}


	// Public Interface Implementations
	inline virtual void updateComponents(const float& deltaTime, const std::vector<std::vector<BaseECSComponent*>>& components) override {
		for each (const auto & componentParam in components) {
			auto* transformComponent = (Transform_Component*)componentParam[0];

			// Reset the world transform to the local transform
			transformComponent->m_worldTransform = transformComponent->m_localTransform;
		}
	}
};

#endif // TRANSFORM_SYSTEM_H