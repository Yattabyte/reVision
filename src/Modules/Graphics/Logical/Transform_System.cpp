#include "Modules/Graphics/Logical/Transform_System.h"
#include "Modules/ECS/component_types.h"
#include "Modules/ECS/ecsWorld.h"


Transform_System::Transform_System(Engine& engine) :
	m_engine(engine)
{
	addComponentType(Transform_Component::Runtime_ID, RequirementsFlag::FLAG_REQUIRED);
}

void Transform_System::updateComponents(const float&, const std::vector<std::vector<ecsBaseComponent*>>& components)
{
	// Reset the world transform to be the local transform of all components
	for (const auto& componentParam : components) {
		auto* transformComponent = static_cast<Transform_Component*>(componentParam[0]);
		transformComponent->m_worldTransform = transformComponent->m_localTransform;
	}

	// Compound the transforms of all parent-child entities
	const std::function<void(const EntityHandle&)> transformHierarchy = [&](const EntityHandle& entityHandle) {
		if (auto* entityTransform = m_world->getComponent<Transform_Component>(entityHandle)) {
			for (const auto& childHandle : m_world->getEntityHandles(entityHandle)) {
				if (auto* childTransform = m_world->getComponent<Transform_Component>(childHandle)) {
					childTransform->m_worldTransform = entityTransform->m_worldTransform * childTransform->m_worldTransform;
					transformHierarchy(childHandle);
				}
			}
		}
	};
	for (const auto& entity : m_world->getEntityHandles())
		transformHierarchy(entity);
}