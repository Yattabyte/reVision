#pragma once
#ifndef DIRECTVISIBILITY_SYSTEM_H
#define DIRECTVISIBILITY_SYSTEM_H

#include "Modules/ECS/ecsSystem.h"
#include "Modules/ECS/component_types.h"
#include "Modules/Graphics/Lighting/Direct/DirectData.h"


/** An ECS system responsible for populating render lists PER active perspective in a given frame, for all light related entities. */
class DirectVisibility_System final : public ecsBaseSystem {
public:
	// Public (De)Constructors
	/** Destroy this system. */
	inline ~DirectVisibility_System() = default;
	/** Construct this system.
	@param	frameData	reference to common data that changes frame-to-frame. */
	inline explicit DirectVisibility_System(Direct_Light_Data& frameData) noexcept :
		m_frameData(frameData)
	{
		addComponentType(Light_Component::Runtime_ID, RequirementsFlag::FLAG_REQUIRED);
	}


	// Public Interface Implementations
	inline virtual void updateComponents(const float& deltaTime, const std::vector<std::vector<ecsBaseComponent*>>& components) noexcept override final {
		// Compile results PER viewport
		for (auto& viewInfo : m_frameData.viewInfo) {
			// Clear previous cached data
			viewInfo.lightIndices.clear();
			viewInfo.lightTypes.clear();

			int index = 0;
			for (const auto& componentParam : components) {
				// Render lights and shadows for all directional lights
				const auto& lightComponent = static_cast<Light_Component*>(componentParam[0]);
				viewInfo.lightIndices.push_back((GLuint)index++);
				viewInfo.lightTypes.push_back(lightComponent->m_type);
			}
		}
	}


private:
	// Private Attributes
	Direct_Light_Data& m_frameData;
};

#endif // DIRECTVISIBILITY_SYSTEM_H
