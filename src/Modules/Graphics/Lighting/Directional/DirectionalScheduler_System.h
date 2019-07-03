#pragma once
#ifndef DIRECTIONALSCHEDULER_SYSTEM_H
#define DIRECTIONALSCHEDULER_SYSTEM_H

#include "Modules/World/ECS/ecsSystem.h"
#include "Modules/Graphics/Lighting/components.h"
#include "Modules/Graphics/Lighting/Directional/DirectionalData.h"
#include "Engine.h"


/***/
class DirectionalScheduler_System : public BaseECSSystem {
public:
	// Public (de)Constructors
	/***/
	inline ~DirectionalScheduler_System() {
		// Update indicator
		m_aliveIndicator = false;
	}
	/***/
	inline DirectionalScheduler_System(Engine * engine, const std::shared_ptr<DirectionalData> & frameData)
		: m_engine(engine), m_frameData(frameData) {
		addComponentType(Renderable_Component::ID, FLAG_REQUIRED);
		addComponentType(LightDirectional_Component::ID, FLAG_REQUIRED);
		addComponentType(CameraArray_Component::ID, FLAG_REQUIRED);

		auto & preferences = engine->getPreferenceState();
		preferences.getOrSetValue(PreferenceState::C_SHADOW_MAX_PER_FRAME, m_maxShadowsCasters);
		preferences.addCallback(PreferenceState::C_SHADOW_MAX_PER_FRAME, m_aliveIndicator, [&](const float &f) { m_maxShadowsCasters = (unsigned int)f; });
	}


	// Public Interface Implementations
	inline virtual void updateComponents(const float & deltaTime, const std::vector<std::vector<BaseECSComponent*>> & components) override {
		// Resize shadowmap to fit number of entities this frame
		m_frameData->shadowFBO.resize(m_frameData->shadowSize, (unsigned int)(components.size() * 4ull));

		if (m_engine->getActionState().isAction(ActionState::FIRE2))
			m_sceneOutOfDate = true;

		// Maintain list of shadows, update with oldest within range
		// Technique will clear list when ready
		int index = 0;
		for each (const auto & componentParam in components) {
			Renderable_Component * renderableComponent = (Renderable_Component*)componentParam[0];
			LightDirectional_Component * lightComponent = (LightDirectional_Component*)componentParam[1];
			CameraArray_Component * cameraComponent = (CameraArray_Component*)componentParam[2];

			// Set appropriate shadow spot
			lightComponent->m_shadowSpot = index * 4;


			if (renderableComponent->m_visibleAtAll && lightComponent->m_hasShadow && m_sceneOutOfDate) {
				bool didAnything = false;
				// Try to find the oldest components
				for (int x = 0; x < m_frameData->shadowsToUpdate.size(); ++x) {
					auto &[oldTime, oldShadowSpot, oldCameras] = m_frameData->shadowsToUpdate.at(x);
					if ((oldShadowSpot != -1 && lightComponent->m_updateTime < *oldTime)) {
						// Shuffle next elements down
						for (auto y = m_frameData->shadowsToUpdate.size() - 1ull; y > x; --y)
							m_frameData->shadowsToUpdate.at(y) = m_frameData->shadowsToUpdate.at(y - 1ull);
						oldTime = &lightComponent->m_updateTime;
						oldShadowSpot = lightComponent->m_shadowSpot;						
						oldCameras = cameraComponent->m_cameras;
						didAnything = true;
						break;
					}
				}
				if (!didAnything && m_frameData->shadowsToUpdate.size() < m_maxShadowsCasters) {
					m_frameData->shadowsToUpdate.push_back({ &lightComponent->m_updateTime, lightComponent->m_shadowSpot, cameraComponent->m_cameras });
					lightComponent->m_outOfDate = true;
				}
			}
			index++;
		}

		if (m_frameData->shadowsToUpdate.size() > m_maxShadowsCasters)
			m_frameData->shadowsToUpdate.resize(m_maxShadowsCasters);
		m_sceneOutOfDate = false;
	}


private:
	// Private Attributes
	Engine * m_engine = nullptr;
	GLuint m_maxShadowsCasters = 1u;
	bool m_sceneOutOfDate = true;
	std::shared_ptr<DirectionalData> m_frameData;
	std::shared_ptr<bool> m_aliveIndicator = std::make_shared<bool>(true);
};

#endif // DIRECTIONALSCHEDULER_SYSTEM_H