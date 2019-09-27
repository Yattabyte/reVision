#pragma once
#ifndef DIRECTIONALVISIBILITY_SYSTEM_H
#define DIRECTIONALVISIBILITY_SYSTEM_H

#include "Modules/ECS/ecsSystem.h"
#include "Modules/ECS/component_types.h"
#include "Modules/Graphics/Lighting/Point/PointData.h"


/** An ECS system responsible for populating render lists PER active perspective in a given frame, for all directional light related entities. */
class DirectionalVisibility_System final : public ecsBaseSystem {
public:
	// Public (de)Constructors
	/** Destroy this system. */
	inline ~DirectionalVisibility_System() = default;
	/** Construct this system.
	@param	frameData	shared pointer of common data that changes frame-to-frame. */
	inline DirectionalVisibility_System(const std::shared_ptr<DirectionalData>& frameData)
		: m_frameData(frameData) {
		addComponentType(LightDirectional_Component::m_ID, FLAG_REQUIRED);
		addComponentType(Shadow_Component::m_ID, FLAG_REQUIRED);
		addComponentType(CameraArray_Component::m_ID, FLAG_REQUIRED);
	}


	// Public Interface Implementations
	inline virtual void updateComponents(const float& deltaTime, const std::vector<std::vector<ecsBaseComponent*>>& components) override final {
		// Compile results PER viewport
		for (int x = 0; x < m_frameData->viewInfo.size(); ++x) {
			auto& viewInfo = m_frameData->viewInfo[x];

			viewInfo.lightIndices.clear();
			viewInfo.visShadowCount = 0;
			int index = 0;
			for each (const auto & componentParam in components) {
				auto* lightComponent = (LightDirectional_Component*)componentParam[0];
				auto* shadowComponent = (Shadow_Component*)componentParam[1];
				auto* cameraComponent = (CameraArray_Component*)componentParam[2];

				// Render lights and shadows for all directional lights
				viewInfo.lightIndices.push_back((GLuint)index);
				viewInfo.visShadowCount++;
				index++;
			}
		}
	}


private:
	// Private Attributes
	std::shared_ptr<DirectionalData> m_frameData;
};

#endif // DIRECTIONALVISIBILITY_SYSTEM_H