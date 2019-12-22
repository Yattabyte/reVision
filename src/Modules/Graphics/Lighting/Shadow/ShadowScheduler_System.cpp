#include "Modules/Graphics/Lighting/Shadow/ShadowScheduler_System.h"
#include "Modules/Graphics/Lighting/Shadow/ShadowData.h"
#include "Modules/ECS/component_types.h"
#include "Engine.h"


ShadowScheduler_System::~ShadowScheduler_System() 
{
	// Update indicator
	*m_aliveIndicator = false;
}

ShadowScheduler_System::ShadowScheduler_System(Engine& engine, ShadowData& frameData) :
	m_engine(engine),
	m_frameData(frameData)
{
	addComponentType(Shadow_Component::Runtime_ID, RequirementsFlag::FLAG_REQUIRED);
	addComponentType(Light_Component::Runtime_ID, RequirementsFlag::FLAG_REQUIRED);

	auto& preferences = engine.getPreferenceState();
	m_maxShadowsCasters = 1u;
	preferences.getOrSetValue(PreferenceState::Preference::C_SHADOW_MAX_PER_FRAME, m_maxShadowsCasters);
	preferences.addCallback(PreferenceState::Preference::C_SHADOW_MAX_PER_FRAME, m_aliveIndicator, [&](const float& f) noexcept { m_maxShadowsCasters = (unsigned int)f; });
}

void ShadowScheduler_System::updateComponents(const float&, const std::vector<std::vector<ecsBaseComponent*>>& components) 
{
	// Maintain list of shadows, update with oldest within range
	// Technique will clear list when ready
	auto& shadows = m_frameData.shadowsToUpdate;
	auto& maxShadows = m_maxShadowsCasters;
	const auto& clientCamera = m_engine.getModule_Graphics().getClientCamera();
	const auto& clientPosition = clientCamera->EyePosition;
	const auto& clientFarPlane = clientCamera->FarPlane;
	const auto clientTime = m_engine.GetSystemTime();
	if (const int availableRoom = (int)m_maxShadowsCasters - (int)m_frameData.shadowsToUpdate.size()) {
		int cameraCount = 0;
		for (const auto& componentParam : components) {
			auto* shadow = static_cast<Shadow_Component*>(componentParam[0]);
			//const auto* light = static_cast<Light_Component*>(componentParam[1]);

			const auto tryToAddShadow = [&shadows, &maxShadows, &clientPosition, &clientFarPlane, &clientTime](const int& shadowSpot, Camera* cb, float* updateTime) {
				const float linDist = glm::distance(clientPosition, cb->getFrustumCenter()) / clientFarPlane;
				const float importance_distance = 1.0f - (linDist * linDist);

				const float linTime = (clientTime - *updateTime) / 5.0f;
				const float importance_time = linTime * linTime;

				const float importance = importance_distance + importance_time * (1.0f - importance_distance);
				bool didAnything = false;
				{
					const auto shadowCount = shadows.size();
					// Try to find the oldest components
					for (int x = 0; x < shadowCount; ++x) {
						auto& [oldImportance, oldTime, oldShadowSpot, oldCamera] = shadows[x];
						if ((oldShadowSpot != -1 && importance > oldImportance)) {
							// Expand container by one
							shadows.resize(shadowCount + 1);
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
				}
				const auto shadowCount = shadows.size();
				if (!didAnything && shadowCount < maxShadows)
					shadows.push_back({ importance, updateTime, shadowSpot, cb });
				if (shadowCount > maxShadows)
					shadows.resize(maxShadows);
			};
			// Set appropriate shadow spot
			const auto shadowCameraCount = shadow->m_cameras.size();
			shadow->m_shadowSpot = cameraCount;
			shadow->m_updateTimes.resize(shadowCameraCount);
			for (int x = 0; x < shadowCameraCount; ++x) {
				shadow->m_cameras[x].setEnabled(false);
				tryToAddShadow(shadow->m_shadowSpot + x, &shadow->m_cameras[x], &shadow->m_updateTimes[x]);
			}
			cameraCount += (int)shadowCameraCount;
		}

		// Enable cameras in final set
		for (auto& [importance, time, shadowSpot, camera] : m_frameData.shadowsToUpdate)
			if (camera != nullptr)
				camera->setEnabled(true);

		// Resize the shadow map to fit number of entities this frame
		m_frameData.shadowFBO.resize(glm::ivec2((int)m_frameData.shadowSize), (unsigned int)(cameraCount));
	}
}