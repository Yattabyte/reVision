#pragma once
#ifndef SPOTVISIBILITY_SYSTEM_H
#define SPOTVISIBILITY_SYSTEM_H

#include "Modules/ECS/ecsSystem.h"
#include "Modules/ECS/component_types.h"
#include "Modules/Graphics/Lighting/Spot/SpotData.h"


/** An ECS system responsible for populating render lists PER active perspective in a given frame, for all spot light related entities. */
class SpotVisibility_System final : public ecsBaseSystem {
public:
	// Public (de)Constructors
	/** Destroy this system. */
	inline ~SpotVisibility_System() = default;
	/** Construct this system.
	@param	frameData	shared pointer of common data that changes frame-to-frame. */
	inline SpotVisibility_System(const std::shared_ptr<SpotData> & frameData)
		: m_frameData(frameData) {
		addComponentType(LightSpot_Component::m_ID, FLAG_REQUIRED);
	}


	// Public Interface Implementations
	inline virtual void updateComponents(const float & deltaTime, const std::vector<std::vector<ecsBaseComponent*>> & components) override final {
		// Compile results PER viewport
		for (int x = 0; x < m_frameData->viewInfo.size(); ++x) {
			auto & viewInfo = m_frameData->viewInfo[x];
			
			viewInfo.lightIndices.clear();
			int index = 0;
			for each (const auto & componentParam in components) {
				auto* lightComponent = (LightPoint_Component*)componentParam[0];

				// Synchronize the component if it is visible
				viewInfo.lightIndices.push_back((GLuint)index);
				index++;
			}
		}
	}


private:
	// Private Attributes
	std::shared_ptr<SpotData> m_frameData;
};

#endif // SPOTVISIBILITY_SYSTEM_H