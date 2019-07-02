#pragma once
#ifndef REFLECTORSCHEDULER_SYSTEM_H
#define REFLECTORSCHEDULER_SYSTEM_H

#include "Modules/World/ECS/ecsSystem.h"
#include "Modules/Graphics/Lighting/components.h"
#include "Modules/Graphics/Lighting/Reflector/ReflectorData.h"
#include "Engine.h"


/***/
class ReflectorScheduler_System : public BaseECSSystem {
public:
	// Public (de)Constructors
	/***/
	inline ~ReflectorScheduler_System() {
		// Update indicator
		m_aliveIndicator = false;
	}
	/***/
	inline ReflectorScheduler_System(Engine * engine, const std::shared_ptr<ReflectorData> & frameData)
		: m_frameData(frameData) {
		addComponentType(Renderable_Component::ID, FLAG_REQUIRED);
		addComponentType(Reflector_Component::ID, FLAG_REQUIRED);
		addComponentType(CameraArray_Component::ID, FLAG_REQUIRED);

		auto & preferences = engine->getPreferenceState();
		preferences.getOrSetValue(PreferenceState::C_SHADOW_MAX_PER_FRAME, m_maxShadowsCasters);
		preferences.addCallback(PreferenceState::C_SHADOW_MAX_PER_FRAME, m_aliveIndicator, [&](const float &f) { m_maxShadowsCasters = (unsigned int)f; });
	}


	// Public Interface Implementations
	inline virtual void updateComponents(const float & deltaTime, const std::vector<std::vector<BaseECSComponent*>> & components) override {
		// Resize shadowmap to fit number of entities this frame
		m_frameData->envmapFBO.resize(m_frameData->envmapSize.x, m_frameData->envmapSize.y, (unsigned int)(components.size() * 6ull));

		// Maintain list of reflectors, update with oldest within range
		// Technique will clear list when ready
		int index = 0;
		for each (const auto & componentParam in components) {
			Renderable_Component * renderableComponent = (Renderable_Component*)componentParam[0];
			Reflector_Component * reflectorComponent = (Reflector_Component*)componentParam[1];
			CameraArray_Component * cameraComponent = (CameraArray_Component*)componentParam[2];

			// Set appropriate shadow spot
			reflectorComponent->m_cubeSpot = index * 6;
			
			if (renderableComponent->m_visibleAtAll && reflectorComponent->m_sceneOutOfDate) {
				bool didAnything = false;
				// Try to find the oldest components
				for (int x = 0; x < m_frameData->reflectorsToUpdate.size(); ++x) {
					auto &[oldTime, oldLight, oldCameras] = m_frameData->reflectorsToUpdate.at(x);
					if (!oldLight || (oldLight && reflectorComponent->m_updateTime < oldTime)) {
						// Shuffle next elements down
						for (auto y = m_frameData->reflectorsToUpdate.size() - 1ull; y > x; --y)
							m_frameData->reflectorsToUpdate.at(y) = m_frameData->reflectorsToUpdate.at(y - 1ull);
						oldLight = reflectorComponent;
						oldTime = reflectorComponent->m_updateTime;
						oldCameras = cameraComponent->m_cameras;
						didAnything = true;
						break;
					}
				}
				if (!didAnything && m_frameData->reflectorsToUpdate.size() < m_maxShadowsCasters)
					m_frameData->reflectorsToUpdate.push_back({ reflectorComponent->m_updateTime, reflectorComponent, cameraComponent->m_cameras });
			}
			index++;
		}

		if (m_frameData->reflectorsToUpdate.size() > m_maxShadowsCasters)
			m_frameData->reflectorsToUpdate.resize(m_maxShadowsCasters);
	}


private:
	// Private Attributes
	GLuint m_maxShadowsCasters = 1u;
	std::shared_ptr<ReflectorData> m_frameData;
	std::shared_ptr<bool> m_aliveIndicator = std::make_shared<bool>(true);
};

#endif // REFLECTORSCHEDULER_SYSTEM_H