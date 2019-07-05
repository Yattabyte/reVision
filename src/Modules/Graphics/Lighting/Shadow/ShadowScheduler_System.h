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
		: m_frameData(frameData) {
		addComponentType(Renderable_Component::ID, FLAG_REQUIRED);
		addComponentType(Shadow_Component::ID, FLAG_REQUIRED);
		addComponentType(Camera_Component::ID, FLAG_OPTIONAL);
		addComponentType(CameraArray_Component::ID, FLAG_OPTIONAL);

		auto & preferences = engine->getPreferenceState();
		preferences.getOrSetValue(PreferenceState::C_SHADOW_MAX_PER_FRAME, m_maxShadowsCasters);
		preferences.addCallback(PreferenceState::C_SHADOW_MAX_PER_FRAME, m_aliveIndicator, [&](const float &f) { m_maxShadowsCasters = (unsigned int)f; });
	}


	// Public Interface Implementations
	inline virtual void updateComponents(const float & deltaTime, const std::vector<std::vector<BaseECSComponent*>> & components) override {
		// Maintain list of shadows, update with oldest within range
		// Technique will clear list when ready
		int cameraCount = 0;
		for each (const auto & componentParam in components) {
			Renderable_Component * renderableComponent = (Renderable_Component*)componentParam[0];
			Shadow_Component * shadowComponent = (Shadow_Component*)componentParam[1];
			Camera_Component * camSingle = (Camera_Component*)componentParam[2];
			CameraArray_Component * camArrays = (CameraArray_Component*)componentParam[3];

			// Set appropriate shadow spot
			shadowComponent->m_shadowSpot = cameraCount;

			// Aggregate camera information
			std::vector<CameraBuffer::CamStruct*> cameras;
			if (camSingle)
				cameras.push_back(&(camSingle->m_camera));
			else if (camArrays) 
				for (int x = 0; x < camArrays->m_cameras.size(); ++x)
					cameras.push_back(&(camArrays->m_cameras[x]));			
			cameraCount += cameras.size();

			if (renderableComponent->m_visibleAtAll && cameras.size()) {
				bool didAnything = false;
				// Try to find the oldest components
				for (int x = 0; x < m_frameData->shadowsToUpdate.size(); ++x) {
					auto &[oldTime, oldShadowSpot, oldCameras] = m_frameData->shadowsToUpdate.at(x);
					if ((oldShadowSpot != -1 && shadowComponent->m_updateTime < *oldTime)) {
						// Shuffle next elements down
						for (auto y = m_frameData->shadowsToUpdate.size() - 1ull; y > x; --y)
							m_frameData->shadowsToUpdate.at(y) = m_frameData->shadowsToUpdate.at(y - 1ull);
						oldTime = &shadowComponent->m_updateTime;
						oldShadowSpot = shadowComponent->m_shadowSpot;
						oldCameras = cameras;
						didAnything = true;
						break;
					}
				}
				if (!didAnything && m_frameData->shadowsToUpdate.size() < m_maxShadowsCasters)
					m_frameData->shadowsToUpdate.push_back({ &shadowComponent->m_updateTime, shadowComponent->m_shadowSpot, cameras });
			}	
		}

		if (m_frameData->shadowsToUpdate.size() > m_maxShadowsCasters)
			m_frameData->shadowsToUpdate.resize(m_maxShadowsCasters);

		// Resize shadowmap to fit number of entities this frame
		m_frameData->shadowFBO.resize(glm::ivec2(m_frameData->shadowSize), (unsigned int)(cameraCount));
	}


private:
	// Private Attributes
	GLuint m_maxShadowsCasters = 1u;
	std::shared_ptr<ShadowData> m_frameData;
	std::shared_ptr<bool> m_aliveIndicator = std::make_shared<bool>(true);
};

#endif // SHADOWSCHEDULER_SYSTEM_H