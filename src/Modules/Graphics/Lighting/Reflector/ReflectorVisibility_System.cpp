#include "Modules/Graphics/Lighting/Reflector/ReflectorVisibility_System.h"
#include "Modules/Graphics/Lighting/Reflector/ReflectorData.h"
#include "Modules/ECS/component_types.h"


ReflectorVisibility_System::ReflectorVisibility_System(ReflectorData& frameData) noexcept :
	m_frameData(frameData)
{
	addComponentType(Reflector_Component::Runtime_ID, RequirementsFlag::FLAG_REQUIRED);
	addComponentType(Transform_Component::Runtime_ID, RequirementsFlag::FLAG_REQUIRED);
}

void ReflectorVisibility_System::updateComponents(const float&, const std::vector<std::vector<ecsBaseComponent*>>& components) 
{
	// Compile results PER viewport
	for (auto& viewInfo : m_frameData.viewInfo) {
		// Clear previous cached data
		viewInfo.lightIndices.clear();

		const auto componentCount = components.size();
		for (GLuint index = 0; index < componentCount; ++index)
			viewInfo.lightIndices.push_back(index);
	}
}