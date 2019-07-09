#pragma once
#ifndef DIRECTIONALVISIBILITY_SYSTEM_H
#define DIRECTIONALVISIBILITY_SYSTEM_H

#include "Modules/World/ECS/ecsSystem.h"
#include "Modules/World/ECS/components.h"
#include "Modules/Graphics/Logical/components.h"
#include "Modules/Graphics/Lighting/components.h"
#include "Modules/Graphics/Lighting/Point/PointData.h"


/***/
class DirectionalVisibility_System : public BaseECSSystem {
public:
	// Public (de)Constructors
	/***/
	inline ~DirectionalVisibility_System() = default;
	/***/
	inline DirectionalVisibility_System(const std::shared_ptr<DirectionalData> & frameData, const std::shared_ptr<std::vector<CameraBuffer::CamStruct*>> & cameras)
		: m_frameData(frameData), m_cameras(cameras) {
		addComponentType(Renderable_Component::ID, FLAG_REQUIRED);
		addComponentType(LightDirectional_Component::ID, FLAG_REQUIRED);
		addComponentType(Shadow_Component::ID, FLAG_REQUIRED);
		addComponentType(CameraArray_Component::ID, FLAG_REQUIRED);
	}


	// Public Interface Implementations
	inline virtual void updateComponents(const float & deltaTime, const std::vector<std::vector<BaseECSComponent*>> & components) override {
		// Link together the dimensions of view info and light buffers to that of the viewport vectors
		m_frameData->viewInfo.resize(m_cameras->size());

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
	std::shared_ptr<std::vector<CameraBuffer::CamStruct*>> m_cameras;
};

#endif // DIRECTIONALVISIBILITY_SYSTEM_H