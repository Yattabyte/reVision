#include "Modules/Graphics/Logical/ReflectorPerspective_System.h"
#include "Modules/ECS/component_types.h"


ReflectorPerspective_System::ReflectorPerspective_System(std::vector<Camera*>& sceneCameras) noexcept :
	m_sceneCameras(sceneCameras)
{
	addComponentType(Reflector_Component::Runtime_ID, RequirementsFlag::FLAG_REQUIRED);
}

void ReflectorPerspective_System::updateComponents(const float& deltaTime, const std::vector<std::vector<ecsBaseComponent*>>& components) noexcept 
{
	for (const auto& componentParam : components) {
		auto* cameraComponent = static_cast<Reflector_Component*>(componentParam[0]);
		for (auto& camera : cameraComponent->m_cameras)
			m_sceneCameras.push_back(&camera);
	}
}