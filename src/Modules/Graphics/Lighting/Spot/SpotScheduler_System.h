#pragma once
#ifndef SPOTSCHEDULER_SYSTEM_H
#define SPOTSCHEDULER_SYSTEM_H

#include "Modules/World/ECS/ecsSystem.h"
#include "Modules/Graphics/Lighting/components.h"
#include "Engine.h"


/***/
class SpotScheduler_System : public BaseECSSystem {
public:
	// Public (de)Constructors
	/***/
	inline ~SpotScheduler_System() {
		// Update indicator
		m_aliveIndicator = false;
	}
	/***/
	inline SpotScheduler_System(Engine * engine, const std::shared_ptr<SpotData> & frameData)
		: m_frameData(frameData) {
		addComponentType(Renderable_Component::ID, FLAG_REQUIRED);
		addComponentType(LightSpot_Component::ID, FLAG_REQUIRED);
		addComponentType(Camera_Component::ID, FLAG_REQUIRED);

		auto & preferences = engine->getPreferenceState();
		preferences.getOrSetValue(PreferenceState::C_SHADOW_MAX_PER_FRAME, m_maxShadowsCasters);
		preferences.addCallback(PreferenceState::C_SHADOW_MAX_PER_FRAME, m_aliveIndicator, [&](const float &f) { m_maxShadowsCasters = (unsigned int)f; });
	}


	// Public Interface Implementations
	inline virtual void updateComponents(const float & deltaTime, const std::vector<std::vector<BaseECSComponent*>> & components) override {
		// Resize shadowmap to fit number of entities this frame
		m_frameData->shadowFBO.resize(m_frameData->shadowSize, (unsigned int)(components.size()));

		// Maintain list of shadows, update with oldest within range
		// Technique will clear list when ready
		int index = 0;
		for each (const auto & componentParam in components) {
			Renderable_Component * renderableComponent = (Renderable_Component*)componentParam[0];
			LightSpot_Component * lightComponent = (LightSpot_Component*)componentParam[1];
			Camera_Component * cameraComponent = (Camera_Component*)componentParam[2];

			// Set appropriate shadow spot
			lightComponent->m_shadowSpot = index;

			if (renderableComponent->m_visibleAtAll && lightComponent->m_hasShadow) {
				bool didAnything = false;
				// Try to find the oldest components
				for (int x = 0; x < m_frameData->shadowsToUpdate.size(); ++x) {
					auto &[oldTime, oldShadowSpot, oldCamera] = m_frameData->shadowsToUpdate.at(x);
					if ((oldShadowSpot != -1 && lightComponent->m_updateTime < *oldTime)) {
						// Shuffle next elements down
						for (auto y = m_frameData->shadowsToUpdate.size() - 1ull; y > x; --y)
							m_frameData->shadowsToUpdate.at(y) = m_frameData->shadowsToUpdate.at(y - 1ull);
						oldTime = &lightComponent->m_updateTime;
						oldShadowSpot = lightComponent->m_shadowSpot;
						oldCamera = cameraComponent->m_camera;
						didAnything = true;
						break;
					}
				}
				if (!didAnything && m_frameData->shadowsToUpdate.size() < m_maxShadowsCasters)
					m_frameData->shadowsToUpdate.push_back({ &lightComponent->m_updateTime, lightComponent->m_shadowSpot, cameraComponent->m_camera });
			}
			index++;
		}

		if (m_frameData->shadowsToUpdate.size() > m_maxShadowsCasters)
			m_frameData->shadowsToUpdate.resize(m_maxShadowsCasters);
	}


private:
	// Private Attributes
	GLuint m_maxShadowsCasters = 1u; 
	std::shared_ptr<SpotData> m_frameData;
	std::shared_ptr<bool> m_aliveIndicator = std::make_shared<bool>(true);
};

#endif // SPOTSCHEDULER_SYSTEM_H