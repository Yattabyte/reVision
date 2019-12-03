#pragma once
#ifndef REFLECTORSCHEDULER_SYSTEM_H
#define REFLECTORSCHEDULER_SYSTEM_H

#include "Modules/ECS/ecsSystem.h"
#include "Modules/ECS/component_types.h"
#include "Modules/Graphics/Lighting/Reflector/ReflectorData.h"
#include "Engine.h"


/** An ECS system responsible for scheduling when reflector related entities should be updated. */
class ReflectorScheduler_System final : public ecsBaseSystem {
public:
	// Public (De)Constructors
	/** Destroy this system. */
	inline ~ReflectorScheduler_System() noexcept {
		// Update indicator
		*m_aliveIndicator = false;
	}
	/** Construct this system.
	@param	engine		reference to the engine to use. 
	@param	frameData	reference to common data that changes frame-to-frame. */
	inline ReflectorScheduler_System(Engine& engine, ReflectorData& frameData) noexcept :
		m_engine(engine),
		m_frameData(frameData)
	{
		addComponentType(Reflector_Component::Runtime_ID, RequirementsFlag::FLAG_REQUIRED);

		auto& preferences = engine.getPreferenceState();
		m_maxReflectionCasters = 1u;
		preferences.getOrSetValue(PreferenceState::Preference::C_ENVMAP_MAX_PER_FRAME, m_maxReflectionCasters);
		preferences.addCallback(PreferenceState::Preference::C_ENVMAP_MAX_PER_FRAME, m_aliveIndicator, [&](const float& f) { m_maxReflectionCasters = (unsigned int)f; });
	}


	// Public Interface Implementations
	inline virtual void updateComponents(const float& deltaTime, const std::vector<std::vector<ecsBaseComponent*>>& components) noexcept override final {
		// Maintain list of reflectors, update with oldest within range
		// Technique will clear list when ready
		auto& reflectors = m_frameData.reflectorsToUpdate;
		auto& maxReflectors = m_maxReflectionCasters;
		const auto& clientCamera = m_engine.getModule_Graphics().getClientCamera();
		const auto& clientPosition = clientCamera->EyePosition;
		const auto& clientFarPlane = clientCamera->FarPlane;
		const auto clientTime = m_engine.getTime();
		if (int availableRoom = (int)m_maxReflectionCasters - (int)m_frameData.reflectorsToUpdate.size()) {
			int cameraCount = 0;
			for (const auto& componentParam : components) {
				auto* reflectorComponent = static_cast<Reflector_Component*>(componentParam[0]);

				auto tryToAddReflector = [&reflectors, &maxReflectors, &clientPosition, &clientFarPlane, &clientTime](const int& reflectorSpot, Camera* cb, float* updateTime) {
					const float linDist = glm::distance(clientPosition, cb->getFrustumCenter()) / clientFarPlane;
					const float importance_distance = 1.0f - (linDist * linDist);

					const float linTime = (clientTime - *updateTime) / 5.0f;
					const float importance_time = linTime * linTime;

					const float importance = importance_distance + importance_time * (1.0f - importance_distance);
					bool didAnything = false;
					// Try to find the oldest components
					for (int x = 0; x < reflectors.size(); ++x) {
						auto& [oldImportance, oldTime, oldReflectorSpot, oldCamera] = reflectors[x];
						if ((oldReflectorSpot != -1 && importance > oldImportance)) {
							// Expand container by one
							reflectors.resize(reflectors.size() + 1);
							// Shuffle next elements down
							for (auto y = reflectors.size() - 1ull; y > x; --y)
								reflectors[y] = reflectors[y - 1ull];
							oldImportance = importance;
							oldTime = updateTime;
							oldReflectorSpot = reflectorSpot;
							oldCamera = cb;
							didAnything = true;
							break;
						}
					}
					if (!didAnything && reflectors.size() < maxReflectors)
						reflectors.push_back({ importance, updateTime, reflectorSpot, cb });
					if (reflectors.size() > maxReflectors)
						reflectors.resize(maxReflectors);
				};
				// Set appropriate reflector spot
				reflectorComponent->m_cubeSpot = cameraCount;
				reflectorComponent->m_updateTimes.resize(reflectorComponent->m_cameras.size());
				for (int x = 0; x < reflectorComponent->m_cameras.size(); ++x) {
					reflectorComponent->m_cameras[x].setEnabled(false);
					tryToAddReflector(reflectorComponent->m_cubeSpot + x, &reflectorComponent->m_cameras[x], &reflectorComponent->m_updateTimes[x]);
				}
				cameraCount += (int)reflectorComponent->m_cameras.size();
			}

			// Enable cameras in final set
			for (auto& [importance, time, reflectorSpot, camera] : m_frameData.reflectorsToUpdate)
				camera->setEnabled(true);

			// Resize the reflector map to fit number of entities this frame
			m_frameData.envmapFBO.resize(m_frameData.envmapSize, (unsigned int)(cameraCount));
			m_frameData.reflectorLayers = cameraCount;
		}
	}


private:
	// Private Attributes
	Engine& m_engine;
	ReflectorData& m_frameData;
	GLuint m_maxReflectionCasters = 1u;
	std::shared_ptr<bool> m_aliveIndicator = std::make_shared<bool>(true);
};

#endif // REFLECTORSCHEDULER_SYSTEM_H
