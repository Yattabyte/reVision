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
	inline ReflectorScheduler_System(Engine * engine, const std::shared_ptr<std::vector<Reflector_Component*>> & reflectorsToUpdate)
		: m_reflectorsToUpdate(reflectorsToUpdate) {
		addComponentType(Renderable_Component::ID, FLAG_REQUIRED);
		addComponentType(Reflector_Component::ID, FLAG_REQUIRED);

		auto & preferences = engine->getPreferenceState();
		preferences.getOrSetValue(PreferenceState::C_SHADOW_MAX_PER_FRAME, m_maxShadowsCasters);
		preferences.addCallback(PreferenceState::C_SHADOW_MAX_PER_FRAME, m_aliveIndicator, [&](const float &f) { m_maxShadowsCasters = (unsigned int)f; });
	}


	// Public Interface Implementations
	inline virtual void updateComponents(const float & deltaTime, const std::vector<std::vector<BaseECSComponent*>> & components) override {
		m_reflectorsToUpdate->clear();
		std::vector<std::pair<float, Reflector_Component*>> oldest(m_maxShadowsCasters, { 0, nullptr });
		int maxElement = -1;
		for each (const auto & componentParam in components) {
			Renderable_Component * renderableComponent = (Renderable_Component*)componentParam[0];
			Reflector_Component * reflectorComponent = (Reflector_Component*)componentParam[1];

			if (renderableComponent->m_visible) {
				// Try to find the oldest components
				int x = 0;
				for (auto &[oldTime, oldLight] : oldest) {
					if (!oldLight || (oldLight && reflectorComponent->m_updateTime < oldTime)) {
						// Shuffle next elements down
						for (int y = m_maxShadowsCasters - 1; y > x; --y)
							oldest[y] = oldest[y - 1];
						oldLight = reflectorComponent;
						oldTime = reflectorComponent->m_updateTime;
						maxElement = x > maxElement ? x : maxElement;
						break;
					}
					++x;
				}
			}
		}

		if (maxElement != -1)
			oldest.resize(maxElement + 1);

		for each (const auto & element in oldest)
			if (element.second)
				m_reflectorsToUpdate->push_back(element.second);
	}


private:
	// Private Attributes
	GLuint m_maxShadowsCasters = 1u;
	std::shared_ptr<std::vector<Reflector_Component*>> m_reflectorsToUpdate;
	std::shared_ptr<bool> m_aliveIndicator = std::make_shared<bool>(true);
};

#endif // REFLECTORSCHEDULER_SYSTEM_H