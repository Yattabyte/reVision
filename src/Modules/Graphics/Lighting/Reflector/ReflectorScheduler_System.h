#pragma once
#ifndef REFLECTORSCHEDULER_SYSTEM_H
#define REFLECTORSCHEDULER_SYSTEM_H

#include "Modules/World/ECS/ecsSystem.h"
#include "Modules/Graphics/Lighting/components.h"
#include "Engine.h"
#include <memory>


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
	inline ReflectorScheduler_System(Engine * engine, const std::shared_ptr<std::vector<std::tuple<float, Reflector_Component*, Viewport_Component*>> > & reflectorsToUpdate)
		: m_reflectorsToUpdate(reflectorsToUpdate) {
		addComponentType(Renderable_Component::ID, FLAG_REQUIRED);
		addComponentType(Reflector_Component::ID, FLAG_REQUIRED);
		addComponentType(Viewport_Component::ID, FLAG_REQUIRED);

		auto & preferences = engine->getPreferenceState();
		preferences.getOrSetValue(PreferenceState::C_SHADOW_MAX_PER_FRAME, m_maxShadowsCasters);
		preferences.addCallback(PreferenceState::C_SHADOW_MAX_PER_FRAME, m_aliveIndicator, [&](const float &f) { m_maxShadowsCasters = (unsigned int)f; });
	}


	// Public Interface Implementations
	inline virtual void updateComponents(const float & deltaTime, const std::vector<std::vector<BaseECSComponent*>> & components) override {
		// Maintain list of reflectors, update with oldest within range
		// Technique will clear list when ready
		for each (const auto & componentParam in components) {
			Renderable_Component * renderableComponent = (Renderable_Component*)componentParam[0];
			Reflector_Component * reflectorComponent = (Reflector_Component*)componentParam[1];
			Viewport_Component * viewportComponent = (Viewport_Component*)componentParam[2];
			
			if (renderableComponent->m_visibleAtAll && reflectorComponent->m_sceneOutOfDate) {
				bool didAnything = false;
				// Try to find the oldest components
				for (int x = 0; x < m_reflectorsToUpdate->size(); ++x) {
					auto &[oldTime, oldLight, oldView] = m_reflectorsToUpdate->at(x);
					if (!oldLight || (oldLight && reflectorComponent->m_updateTime < oldTime)) {
						// Shuffle next elements down
						for (int y = m_reflectorsToUpdate->size() - 1; y > x; --y)
							m_reflectorsToUpdate->at(y) = m_reflectorsToUpdate->at(y - 1);
						oldLight = reflectorComponent;
						oldTime = reflectorComponent->m_updateTime;
						oldView = viewportComponent;
						didAnything = true;
						break;
					}
				}
				if (!didAnything && m_reflectorsToUpdate->size() < m_maxShadowsCasters)
					m_reflectorsToUpdate->push_back({ reflectorComponent->m_updateTime, reflectorComponent, viewportComponent });
			}
		}

		if (m_reflectorsToUpdate->size() > m_maxShadowsCasters)
			m_reflectorsToUpdate->resize(m_maxShadowsCasters);
	}


private:
	// Private Attributes
	GLuint m_maxShadowsCasters = 1u;
	std::shared_ptr<std::vector<std::tuple<float, Reflector_Component*, Viewport_Component*>>> m_reflectorsToUpdate;
	std::shared_ptr<bool> m_aliveIndicator = std::make_shared<bool>(true);
};

#endif // REFLECTORSCHEDULER_SYSTEM_H