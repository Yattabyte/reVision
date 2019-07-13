#pragma once
#ifndef SHADOWSCHEDULER_SYSTEM_H
#define SHADOWSCHEDULER_SYSTEM_H

#include "Modules/World/ECS/ecsSystem.h"
#include "Modules/Graphics/Logical/components.h"
#include "Modules/Graphics/Lighting/components.h"
#include "Modules/Graphics/Lighting/Shadow/ShadowData.h"
#include "Engine.h"


/***/
class ShadowScheduler_System : public BaseECSSystem {
public:
	// Public (de)Constructors
	/***/
	inline ~ShadowScheduler_System() {
		// Update indicator
		m_aliveIndicator = false;
	}
	/***/
	inline ShadowScheduler_System(Engine * engine, const std::shared_ptr<ShadowData> & frameData)
		: m_engine(engine), m_frameData(frameData) {
		addComponentType(Renderable_Component::ID, FLAG_REQUIRED);
		addComponentType(Shadow_Component::ID, FLAG_REQUIRED);
		addComponentType(Camera_Component::ID, FLAG_OPTIONAL);
		addComponentType(CameraArray_Component::ID, FLAG_OPTIONAL);

		auto & preferences = engine->getPreferenceState();
		m_maxShadowsCasters = 1u;
		preferences.getOrSetValue(PreferenceState::C_SHADOW_MAX_PER_FRAME, m_maxShadowsCasters);
		preferences.addCallback(PreferenceState::C_SHADOW_MAX_PER_FRAME, m_aliveIndicator, [&](const float &f) { m_maxShadowsCasters = (unsigned int)f; });
	}


	// Public Interface Implementations
	inline virtual void updateComponents(const float & deltaTime, const std::vector<std::vector<BaseECSComponent*>> & components) override {
		// Maintain list of shadows, update with oldest within range
		// Technique will clear list when ready
		auto & shadows = m_frameData->shadowsToUpdate;
		auto & maxShadows = m_maxShadowsCasters;
		auto clientPosition = m_engine->getModule_Graphics().getClientCamera()->get()->EyePosition;
		auto clientFarPlane = m_engine->getModule_Graphics().getClientCamera()->get()->FarPlane;
		const auto clientTime = m_engine->getTime();
		if (int availableRoom = (int)m_maxShadowsCasters - (int)m_frameData->shadowsToUpdate.size()) {
			int cameraCount = 0;
			for each (const auto & componentParam in components) {
				Renderable_Component * renderableComponent = (Renderable_Component*)componentParam[0];
				Shadow_Component * shadowComponent = (Shadow_Component*)componentParam[1];
				Camera_Component * camSingle = (Camera_Component*)componentParam[2];
				CameraArray_Component * camArrays = (CameraArray_Component*)componentParam[3];

				if (renderableComponent->m_visibleAtAll) {
					auto tryToAddShadow = [&shadows, &maxShadows, &clientPosition, &clientFarPlane, &clientTime](const int & shadowSpot, Camera * cb, float * updateTime) {
						const float linDist = glm::distance(clientPosition, cb->getFrustumCenter()) / clientFarPlane;
						const float importance_distance = 1.0f - (linDist * linDist);

						const float linTime = (clientTime - *updateTime) / 5.0f;
						const float importance_time = linTime * linTime;

						const float importance = importance_distance + importance_time * (1.0f - importance_distance);
						bool didAnything = false;
						// Try to find the oldest components
						for (int x = 0; x < shadows.size(); ++x) {
							auto &[oldImportance, oldTime, oldShadowSpot, oldCamera] = shadows[x];
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
					shadowComponent->m_shadowSpot = cameraCount;

					if (camSingle) {
						camSingle->m_camera.setEnabled(false);
						tryToAddShadow(shadowComponent->m_shadowSpot, &(camSingle->m_camera), &camSingle->m_updateTime);
						cameraCount++;
					}
					else if (camArrays) {
						camArrays->m_updateTimes.resize(camArrays->m_cameras.size());
						for (int x = 0; x < camArrays->m_cameras.size(); ++x) {
							camArrays->m_cameras[x].setEnabled(false);
							tryToAddShadow(shadowComponent->m_shadowSpot + x, &(camArrays->m_cameras[x]), &camArrays->m_updateTimes[x]);
						}
						cameraCount += camArrays->m_cameras.size();
					}
				}
			}

			// Enable cameras in final set
			for (auto &[importance, time, shadowSpot, camera] : m_frameData->shadowsToUpdate)
				camera->setEnabled(true);

			// Resize shadowmap to fit number of entities this frame
			m_frameData->shadowFBO.resize(glm::ivec2(m_frameData->shadowSize), (unsigned int)(cameraCount));
		}
	}


private:
	// Private Attributes
	Engine * m_engine = nullptr;
	GLuint m_maxShadowsCasters = 1u;
	std::shared_ptr<ShadowData> m_frameData;
	std::shared_ptr<bool> m_aliveIndicator = std::make_shared<bool>(true);
};

#endif // SHADOWSCHEDULER_SYSTEM_H