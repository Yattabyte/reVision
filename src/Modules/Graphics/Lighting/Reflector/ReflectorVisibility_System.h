#pragma once
#ifndef REFLECTORVISIBILITY_SYSTEM_H
#define REFLECTORVISIBILITY_SYSTEM_H

#include "Modules/ECS/ecsSystem.h"
#include "Modules/ECS/component_types.h"
#include "Modules/Graphics/Lighting/Reflector/ReflectorData.h"


/** An ECS system responsible for populating render lists PER active perspective in a given frame, for all reflector related entities. */
class ReflectorVisibility_System final : public ecsBaseSystem {
public:
	// Public (De)Constructors
	/** Destroy this system. */
	inline ~ReflectorVisibility_System() = default;
	/** Construct this system.
	@param	frameData	reference to common data that changes frame-to-frame. */
	inline explicit ReflectorVisibility_System(ReflectorData& frameData) noexcept :
		m_frameData(frameData)
	{
		addComponentType(Reflector_Component::Runtime_ID, RequirementsFlag::FLAG_REQUIRED);
		addComponentType(Transform_Component::Runtime_ID, RequirementsFlag::FLAG_REQUIRED);
	}


	// Public Interface Implementations
	inline virtual void updateComponents(const float& deltaTime, const std::vector<std::vector<ecsBaseComponent*>>& components) noexcept override final {
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


private:
	// Private Attributes
	ReflectorData& m_frameData;
};

#endif // REFLECTORVISIBILITY_SYSTEM_H
