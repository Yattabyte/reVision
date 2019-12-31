#include "Modules/Graphics/Lighting/Direct/DirectVisibility_System.h"
#include "Modules/Graphics/Lighting/Direct/DirectData.h"
#include "Modules/ECS/component_types.h"


DirectVisibility_System::DirectVisibility_System(Direct_Light_Data& frameData) :
	m_frameData(frameData)
{
	addComponentType(Light_Component::Runtime_ID, RequirementsFlag::FLAG_REQUIRED);
}

void DirectVisibility_System::updateComponents(const float& /*deltaTime*/, const std::vector<std::vector<ecsBaseComponent*>>& components)
{
	// Compile results PER viewport
	for (auto& viewInfo : m_frameData.viewInfo) {
		// Clear previous cached data
		viewInfo.lightIndices.clear();
		viewInfo.lightTypes.clear();

		int index = 0;
		for (const auto& componentParam : components) {
			// Render lights and shadows for all directional lights
			const auto& lightComponent = static_cast<Light_Component*>(componentParam[0]);
			viewInfo.lightIndices.push_back(static_cast<GLuint>(index++));
			viewInfo.lightTypes.push_back(lightComponent->m_type);
		}
	}
}