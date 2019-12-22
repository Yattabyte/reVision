#include "Modules/Graphics/Logical/ShadowPerspective_System.h"
#include "Modules/ECS/component_types.h"


ShadowPerspective_System::ShadowPerspective_System(std::vector<Camera*>& sceneCameras) :
	m_sceneCameras(sceneCameras)
{
	addComponentType(Shadow_Component::Runtime_ID, RequirementsFlag::FLAG_REQUIRED);
}

void ShadowPerspective_System::updateComponents(const float&, const std::vector<std::vector<ecsBaseComponent*>>& components) {
	for (const auto& componentParam : components) {
		auto* shadow = static_cast<Shadow_Component*>(componentParam[0]);
		for (auto& camera : shadow->m_cameras)
			m_sceneCameras.push_back(&camera);
	}
}