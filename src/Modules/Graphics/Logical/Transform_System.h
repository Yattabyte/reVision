#pragma once
#ifndef TRANSFORM_SYSTEM_H
#define TRANSFORM_SYSTEM_H

#include "Modules/ECS/ecsSystem.h"
#include "Modules/ECS/component_types.h"
#include "glm/gtc/type_ptr.hpp"
#include "Engine.h"
#include <memory>


/** An ECS system responsible for applying a transformation hierarchy between parent and child entities. */
class Transform_System final : public ecsBaseSystem {
public:
	// Public (de)Constructors
	/** Destroy this system. */
	inline ~Transform_System() = default;
	/** Construct this system.
	@param	engine		the currently active engine. */
	inline Transform_System(Engine * engine)
		: m_engine(engine) {
		addComponentType(Transform_Component::m_ID, FLAG_REQUIRED);
	}


	// Public Interface Implementations
	inline virtual void updateComponents(const float& deltaTime, const std::vector<std::vector<ecsBaseComponent*>>& components) override final {
		// Reset the world transform to be the local transform of all components
		for each (const auto & componentParam in components) {
			auto* transformComponent = (Transform_Component*)componentParam[0];
			transformComponent->m_worldTransform = transformComponent->m_localTransform;
		}

		// Compound the transforms of all parent-child entities
		auto& ecsWorld = m_engine->getModule_ECS().getWorld();
		const std::function<void(const ecsHandle&)> transformHierarchy = [&](const ecsHandle& entityHandle) {
			if (auto * entityTransform = ecsWorld.getComponent<Transform_Component>(entityHandle)) {
				for each (const auto & childHandle in ecsWorld.getEntityHandles(entityHandle)) {
					if (auto * childTransform = ecsWorld.getComponent<Transform_Component>(childHandle)) {
						childTransform->m_worldTransform = entityTransform->m_worldTransform * childTransform->m_worldTransform;
						transformHierarchy(childHandle);
					}
				}
			}
		};
		for each (const auto & entity in ecsWorld.getEntityHandles())
			transformHierarchy(entity);
	}


private:
	// Private Attributes
	Engine * m_engine = nullptr;
};

#endif // TRANSFORM_SYSTEM_H