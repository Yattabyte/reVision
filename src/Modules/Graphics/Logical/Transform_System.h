#pragma once
#ifndef TRANSFORM_SYSTEM_H
#define TRANSFORM_SYSTEM_H

#include "Modules/World/ECS/ecsSystem.h"
#include "Modules/World/ECS/components.h"
#include "glm/gtc/type_ptr.hpp"
#include "Engine.h"
#include <memory>


/** An ECS system responsible for applying a transformation hierarchy between parent and child entities. */
class Transform_System : public BaseECSSystem {
public:
	// Public (de)Constructors
	/** Destroy this system. */
	inline ~Transform_System() = default;
	/** Construct this system.
	@param	engine		the currently active engine. */
	inline Transform_System(Engine * engine)
		: m_engine(engine) {
		addComponentType(Transform_Component::ID, FLAG_REQUIRED);
	}


	// Public Interface Implementations
	inline virtual void updateComponents(const float& deltaTime, const std::vector<std::vector<BaseECSComponent*>>& components) override {
		// Reset the world transform to be the local transform of all components
		for each (const auto & componentParam in components) {
			auto* transformComponent = (Transform_Component*)componentParam[0];
			transformComponent->m_worldTransform = transformComponent->m_localTransform;
		}

		// Compound the transforms of all parent-child entities
		auto& world = m_engine->getModule_World();
		const std::function<void(ecsEntity*)> transformHierarchy = [&](ecsEntity * entity) {
			if (auto * entityTransform = world.getComponent<Transform_Component>(entity->m_uuid)) {
				for each (const auto & child in entity->m_children) {
					if (auto * childTransform = world.getComponent<Transform_Component>(child->m_uuid)) {
						childTransform->m_worldTransform = entityTransform->m_worldTransform * childTransform->m_worldTransform;
						transformHierarchy(child);
					}
				}
			}
		};
		for each (const auto & entity in world.getEntities())
			transformHierarchy(entity);
	}


private:
	// Private Attributes
	Engine * m_engine = nullptr;
};

#endif // TRANSFORM_SYSTEM_H