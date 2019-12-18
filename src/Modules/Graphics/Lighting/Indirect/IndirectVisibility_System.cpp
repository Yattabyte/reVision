#include "Modules/Graphics/Lighting/Indirect/IndirectVisibility_System.h"
#include "Modules/Graphics/Lighting/Indirect/IndirectData.h"
#include "Modules/ECS/component_types.h"


IndirectVisibility_System::IndirectVisibility_System(Indirect_Light_Data& frameData) noexcept :
	m_frameData(frameData)
{
	addComponentType(Light_Component::Runtime_ID, RequirementsFlag::FLAG_REQUIRED);
	addComponentType(Shadow_Component::Runtime_ID, RequirementsFlag::FLAG_REQUIRED);
}

void IndirectVisibility_System::updateComponents(const float&, const std::vector<std::vector<ecsBaseComponent*>>& components) 
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