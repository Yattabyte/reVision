#pragma once
#ifndef SHADOWSCHEDULER_SYSTEM_H
#define SHADOWSCHEDULER_SYSTEM_H

#include "Modules/ECS/ecsSystem.h"
#include "Modules/ECS/component_types.h"
#include "Modules/Graphics/Lighting/Shadow/ShadowData.h"
#include "Engine.h"


/** An ECS system responsible for scheduling when light & shadow related entities should be updated. */
class ShadowScheduler_System final : public ecsBaseSystem {
public:
	// Public (De)Constructors
	/** Destroy this system. */
	inline ~ShadowScheduler_System() {
		// Update indicator
		*m_aliveIndicator = false;
	}
	/** Construct this system.
	@param	engine		the engine to use.
	@param	frameData	shared pointer of common data that changes frame-to-frame. */
	inline ShadowScheduler_System(Engine* engine, const std::shared_ptr<ShadowData>& frameData)
		: m_engine(engine), m_frameData(frameData) {
		addComponentType(Shadow_Component::Runtime_ID, FLAG_REQUIRED);
		addComponentType(Light_Component::Runtime_ID, FLAG_REQUIRED);

		auto& preferences = engine->getPreferenceState();
		m_maxShadowsCasters = 1u;
		preferences.getOrSetValue(PreferenceState::C_SHADOW_MAX_PER_FRAME, m_maxShadowsCasters);
		preferences.addCallback(PreferenceState::C_SHADOW_MAX_PER_FRAME, m_aliveIndicator, [&](const float& f) { m_maxShadowsCasters = (unsigned int)f; });
	}


	// Public Interface Implementations
	inline virtual void updateComponents(const float& deltaTime, const std::vector<std::vector<ecsBaseComponent*>>& components) override final {
		// Maintain list of shadows, update with oldest within range
		// Technique will clear list when ready
		auto& shadows = m_frameData->shadowsToUpdate;
		auto& maxShadows = m_maxShadowsCasters;
		auto clientPosition = m_engine->getModule_Graphics().getClientCamera()->get()->EyePosition;
		auto clientFarPlane = m_engine->getModule_Graphics().getClientCamera()->get()->FarPlane;
		const auto clientTime = m_engine->getTime();
		if (int availableRoom = (int)m_maxShadowsCasters - (int)m_frameData->shadowsToUpdate.size()) {
			int cameraCount = 0;
			for each (const auto & componentParam in components) {
				auto* shadow = static_cast<Shadow_Component*>(componentParam[0]);
				//const auto* light = static_cast<Light_Component*>(componentParam[1]);

				auto tryToAddShadow = [&shadows, &maxShadows, &clientPosition, &clientFarPlane, &clientTime](const int& shadowSpot, Camera* cb, float* updateTime) {
					const float linDist = glm::distance(clientPosition, cb->getFrustumCenter()) / clientFarPlane;
					const float importance_distance = 1.0f - (linDist * linDist);

					const float linTime = (clientTime - *updateTime) / 5.0f;
					const float importance_time = linTime * linTime;

					const float importance = importance_distance + importance_time * (1.0f - importance_distance);
					bool didAnything = false;
					// Try to find the oldest components
					for (int x = 0; x < shadows.size(); ++x) {
						auto& [oldImportance, oldTime, oldShadowSpot, oldCamera] = shadows[x];
						if ((oldShadowSpot != -1 && importance > oldImportance)) {
							// Expand container by one
							shadows.resize(shadows.size() + 1);
							// Shuffle next elements down
							for (auto y = shadows.size() - 1ull; y > x; --y)
								shadows[y] = shadows[y - 1ull];
							oldImportance = importance;
							oldTime = updateTime;
							oldShadowSpot = shadowSpot;
							oldCamera = cb;
							didAnything = true;
							break;
						}
					}
					if (!didAnything && shadows.size() < maxShadows)
						shadows.push_back({ importance, updateTime, shadowSpot, cb });
					if (shadows.size() > maxShadows)
						shadows.resize(maxShadows);
				};
				// Set appropriate shadow spot
				shadow->m_shadowSpot = cameraCount;
				shadow->m_updateTimes.resize(shadow->m_cameras.size());
				for (int x = 0; x < shadow->m_cameras.size(); ++x) {
					shadow->m_cameras[x].setEnabled(false);
					tryToAddShadow(shadow->m_shadowSpot + x, &shadow->m_cameras[x], &shadow->m_updateTimes[x]);
				}
				cameraCount += (int)shadow->m_cameras.size();
			}

			// Enable cameras in final set
			for (auto& [importance, time, shadowSpot, camera] : m_frameData->shadowsToUpdate)
				camera->setEnabled(true);

			// Resize the shadow map to fit number of entities this frame
			m_frameData->shadowFBO.resize(glm::ivec2((int)m_frameData->shadowSize), (unsigned int)(cameraCount));
		}
	}


private:
	// Private Attributes
	Engine* m_engine = nullptr;
	GLuint m_maxShadowsCasters = 1u;
	std::shared_ptr<ShadowData> m_frameData;
	std::shared_ptr<bool> m_aliveIndicator = std::make_shared<bool>(true);
};

#endif // SHADOWSCHEDULER_SYSTEM_H