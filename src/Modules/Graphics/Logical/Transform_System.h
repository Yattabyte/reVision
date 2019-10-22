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
	inline explicit Transform_System(Engine* engine)
		: m_engine(engine) {
		addComponentType(Transform_Component::m_ID, FLAG_REQUIRED);
	}


	// Public Interface Implementations
	inline virtual void updateComponents(const float& deltaTime, const std::vector<std::vector<ecsBaseComponent*>>& components) override final {
		// Reset the world transform to be the local transform of all components
		for each (const auto & componentParam in components) {
			auto* transformComponent = static_cast<Transform_Component*>(componentParam[0]);
			transformComponent->m_worldTransform = transformComponent->m_localTransform;
		}

		// Compound the transforms of all parent-child entities
		const std::function<void(const EntityHandle&)> transformHierarchy = [&](const EntityHandle& entityHandle) {
			if (auto* entityTransform = m_world->getComponent<Transform_Component>(entityHandle)) {
				for each (const auto & childHandle in m_world->getEntityHandles(entityHandle)) {
					if (auto* childTransform = m_world->getComponent<Transform_Component>(childHandle)) {
						childTransform->m_worldTransform = entityTransform->m_worldTransform * childTransform->m_worldTransform;
						transformHierarchy(childHandle);
					}
				}
			}
		};
		for each (const auto & entity in m_world->getEntityHandles())
			transformHierarchy(entity);
	}


	// Public Attributes
	Engine* m_engine = nullptr;
	ecsWorld* m_world = nullptr;
};

#endif // TRANSFORM_SYSTEM_H