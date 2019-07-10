#pragma once
#ifndef SPOTVISIBILITY_SYSTEM_H
#define SPOTVISIBILITY_SYSTEM_H

#include "Modules/World/ECS/ecsSystem.h"
#include "Modules/World/ECS/components.h"
#include "Modules/Graphics/Logical/components.h"
#include "Modules/Graphics/Lighting/components.h"
#include "Modules/Graphics/Lighting/Spot/SpotData.h"


/***/
class SpotVisibility_System : public BaseECSSystem {
public:
	// Public (de)Constructors
	/***/
	inline ~SpotVisibility_System() = default;
	/***/
	inline SpotVisibility_System(const std::shared_ptr<SpotData> & frameData)
		: m_frameData(frameData) {
		addComponentType(Renderable_Component::ID, FLAG_REQUIRED);
		addComponentType(LightSpot_Component::ID, FLAG_REQUIRED);
	}


	// Public Interface Implementations
	inline virtual void updateComponents(const float & deltaTime, const std::vector<std::vector<BaseECSComponent*>> & components) override {
		// Compile results PER viewport
		for (int x = 0; x < m_frameData->viewInfo.size(); ++x) {
			auto & viewInfo = m_frameData->viewInfo[x];
			
			viewInfo.lightIndices.clear();
			int index = 0;
			for each (const auto & componentParam in components) {
				Renderable_Component * renderableComponent = (Renderable_Component*)componentParam[0];
				LightPoint_Component * lightComponent = (LightPoint_Component*)componentParam[1];

				// Synchronize the component if it is visible
				if (renderableComponent->m_visible[x])
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