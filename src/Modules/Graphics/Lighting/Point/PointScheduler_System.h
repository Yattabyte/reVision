#pragma once
#ifndef POINTSCHEDULER_SYSTEM_H
#define POINTSCHEDULER_SYSTEM_H

#include "Modules/World/ECS/ecsSystem.h"
#include "Modules/Graphics/Lighting/components.h"
#include "Modules/Graphics/Lighting/Point/PointData.h"
#include "Engine.h"


/***/
class PointScheduler_System : public BaseECSSystem {
public:
	// Public (de)Constructors
	/***/
	inline ~PointScheduler_System() {
		// Update indicator
		m_aliveIndicator = false;
	}
	/***/
	inline PointScheduler_System(Engine * engine, const std::shared_ptr<PointData> & frameData)
		: m_frameData(frameData) {
		addComponentType(Renderable_Component::ID, FLAG_REQUIRED);
		addComponentType(LightPoint_Component::ID, FLAG_REQUIRED);
		addComponentType(CameraArray_Component::ID, FLAG_REQUIRED);

		auto & preferences = engine->getPreferenceState();
		preferences.getOrSetValue(PreferenceState::C_SHADOW_MAX_PER_FRAME, m_maxShadowsCasters);
		preferences.addCallback(PreferenceState::C_SHADOW_MAX_PER_FRAME, m_aliveIndicator, [&](const float &f) { m_maxShadowsCasters = (unsigned int)f; });
	}


	// Public Interface Implementations
	inline virtual void updateComponents(const float & deltaTime, const std::vector<std::vector<BaseECSComponent*>> & components) override {
		// Resize shadowmap to fit number of entities this frame
		m_frameData->shadowFBO.resize(m_frameData->shadowSize, components.size() * 6);

		// Maintain list of shadows, update with oldest within range
		// Technique will clear list when ready
		int index = 0;
		for each (const auto & componentParam in components) {
			Renderable_Component * renderableComponent = (Renderable_Component*)componentParam[0];
			LightPoint_Component * lightComponent = (LightPoint_Component*)componentParam[1];
			CameraArray_Component * cameraComponent = (CameraArray_Component*)componentParam[2];

			// Set appropriate shadow spot
			lightComponent->m_shadowSpot = index * 6;

			if (renderableComponent->m_visibleAtAll && lightComponent->m_hasShadow) {
				bool didAnything = false;
				// Try to find the oldest components
				for (int x = 0; x < m_frameData->shadowsToUpdate.size(); ++x) {
					auto &[oldTime, oldLight, oldCameras] = m_frameData->shadowsToUpdate.at(x);
					if (!oldLight || (oldLight && lightComponent->m_updateTime < oldTime)) {
						// Shuffle next elements down
						for (int y = m_frameData->shadowsToUpdate.size() - 1; y > x; --y)
							m_frameData->shadowsToUpdate.at(y) = m_frameData->shadowsToUpdate.at(y - 1);
						oldLight = lightComponent;
						oldTime = lightComponent->m_updateTime;
						oldCameras = cameraComponent->m_cameras;
						didAnything = true;
						break;
					}
				}
				if (!didAnything && m_frameData->shadowsToUpdate.size() < m_maxShadowsCasters)
					m_frameData->shadowsToUpdate.push_back({ lightComponent->m_updateTime, lightComponent, cameraComponent->m_cameras });
			}
			index++;
		}

		if (m_frameData->shadowsToUpdate.size() > m_maxShadowsCasters)
			m_frameData->shadowsToUpdate.resize(m_maxShadowsCasters);
	}


private:
	// Private Attributes
	GLuint m_maxShadowsCasters = 1u;
	std::shared_ptr<PointData> m_frameData;
	std::shared_ptr<bool> m_aliveIndicator = std::make_shared<bool>(true);
};

#endif // POINTSCHEDULER_SYSTEM_H