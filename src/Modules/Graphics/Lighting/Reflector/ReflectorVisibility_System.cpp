#include "Modules/Graphics/Lighting/Reflector/ReflectorVisibility_System.h"
#include "Modules/Graphics/Lighting/Reflector/ReflectorData.h"
#include "Modules/ECS/component_types.h"


ReflectorVisibility_System::ReflectorVisibility_System(ReflectorData& frameData) noexcept :
	m_frameData(frameData)
{
	addComponentType(Reflector_Component::Runtime_ID, RequirementsFlag::FLAG_REQUIRED);
	addComponentType(Transform_Component::Runtime_ID, RequirementsFlag::FLAG_REQUIRED);
}

void ReflectorVisibility_System::updateComponents(const float&, const std::vector<std::vector<ecsBaseComponent*>>& components) noexcept 
{
	// Compile results PER viewport
	for (int x = 0; x < m_frameData.viewInfo.size(); ++x) {
		auto& viewInfo = m_frameData.viewInfo[x];

		viewInfo.lightIndices.clear();

		//for (const auto& componentParam : components)
		for (GLuint index = 0; index < components.size(); ++index)
			viewInfo.lightIndices.push_back(index);
	}
}