#include "Modules/Graphics/Logical/CameraPerspective_System.h"


CameraPerspective_System::CameraPerspective_System(std::vector<Camera*>& sceneCameras) noexcept :
	m_sceneCameras(sceneCameras)
{
	addComponentType(Camera_Component::Runtime_ID, RequirementsFlag::FLAG_REQUIRED);
}

void CameraPerspective_System::updateComponents(const float& deltaTime, const std::vector<std::vector<ecsBaseComponent*>>& components) noexcept 
{
	for (const auto& componentParam : components) {
		auto* cameraComponent = static_cast<Camera_Component*>(componentParam[0]);
		m_sceneCameras.push_back(&cameraComponent->m_camera);
	}
}
