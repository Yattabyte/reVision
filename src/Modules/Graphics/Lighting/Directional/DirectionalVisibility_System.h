#pragma once
#ifndef DIRECTIONALVISIBILITY_SYSTEM_H
#define DIRECTIONALVISIBILITY_SYSTEM_H

#include "Modules/World/ECS/ecsSystem.h"
#include "Modules/World/ECS/components.h"
#include "Modules/Graphics/Lighting/Point/PointData.h"


/** An ECS system responsible for populating render lists PER active perspective in a given frame, for all directional light related entities. */
class DirectionalVisibility_System : public BaseECSSystem {
public:
	// Public (de)Constructors
	/** Destroy this system. */
	inline ~DirectionalVisibility_System() = default;
	/** Construct this system.
	@param	frameData	shared pointer of common data that changes frame-to-frame. */
	inline DirectionalVisibility_System(const std::shared_ptr<DirectionalData> & frameData)
		: m_frameData(frameData) {
		addComponentType(Renderable_Component::ID, FLAG_REQUIRED);
		addComponentType(LightDirectional_Component::ID, FLAG_REQUIRED);
		addComponentType(Shadow_Component::ID, FLAG_REQUIRED);
		addComponentType(CameraArray_Component::ID, FLAG_REQUIRED);
	}


	// Public Interface Implementations
	inline virtual void updateComponents(const float & deltaTime, const std::vector<std::vector<BaseECSComponent*>> & components) override {
		// Compile results PER viewport
		for (int x = 0; x < m_frameData->viewInfo.size(); ++x) {
			auto & viewInfo = m_frameData->viewInfo[x];

			viewInfo.lightIndices.clear();
			viewInfo.visShadowCount = 0;
			int index = 0;
			for each (const auto & componentParam in components) {
				Renderable_Component * renderableComponent = (Renderable_Component*)componentParam[0];
				LightDirectional_Component * lightComponent = (LightDirectional_Component*)componentParam[1];
				Shadow_Component * shadowComponent = (Shadow_Component*)componentParam[2];
				CameraArray_Component * cameraComponent = (CameraArray_Component*)componentParam[3];

				// Render lights and shadows for all visible directional lights
				if (renderableComponent->m_visible[x])
					viewInfo.lightIndices.push_back((GLuint)index);

				if (renderableComponent->m_visibleAtAll)
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