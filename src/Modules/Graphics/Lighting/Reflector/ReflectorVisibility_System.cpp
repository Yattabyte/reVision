#include "Modules/Graphics/Lighting/Reflector/ReflectorVisibility_System.h"


ReflectorVisibility_System::ReflectorVisibility_System(ReflectorData& frameData) noexcept :
	m_frameData(frameData)
{
	addComponentType(Reflector_Component::Runtime_ID, RequirementsFlag::FLAG_REQUIRED);
	addComponentType(Transform_Component::Runtime_ID, RequirementsFlag::FLAG_REQUIRED);
}

void ReflectorVisibility_System::updateComponents(const float& deltaTime, const std::vector<std::vector<ecsBaseComponent*>>& components) noexcept 
{
	// Compile results PER viewport
	for (int x = 0; x < m_frameData.viewInfo.size(); ++x) {
		auto& viewInfo = m_frameData.viewInfo[x];

		viewInfo.lightIndices.clear();
		int index = 0;
		for (const auto& componentParam : components) {
			//const auto* reflectorComponent = static_cast<Reflector_Component*>(componentParam[0]);
			//const auto* transformComponent = static_cast<Transform_Component*>(componentParam[1]);

			// Synchronize the component if it is visible
			viewInfo.lightIndices.push_back((GLuint)index);
			index++;
		}
	}
}