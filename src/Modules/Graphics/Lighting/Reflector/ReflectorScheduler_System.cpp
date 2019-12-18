#include "Modules/Graphics/Lighting/Reflector/ReflectorScheduler_System.h"
#include "Modules/Graphics/Lighting/Reflector/ReflectorData.h"
#include "Modules/ECS/component_types.h"
#include "Engine.h"


ReflectorScheduler_System::~ReflectorScheduler_System() noexcept
{
	// Update indicator
	*m_aliveIndicator = false;
}

ReflectorScheduler_System::ReflectorScheduler_System(Engine& engine, ReflectorData& frameData) :
	m_engine(engine),
	m_frameData(frameData)
{
	addComponentType(Reflector_Component::Runtime_ID, RequirementsFlag::FLAG_REQUIRED);

	auto& preferences = engine.getPreferenceState();
	m_maxReflectionCasters = 1u;
	preferences.getOrSetValue(PreferenceState::Preference::C_ENVMAP_MAX_PER_FRAME, m_maxReflectionCasters);
	preferences.addCallback(PreferenceState::Preference::C_ENVMAP_MAX_PER_FRAME, m_aliveIndicator, [&](const float& f) noexcept { m_maxReflectionCasters = (unsigned int)f; });
}

void ReflectorScheduler_System::updateComponents(const float&, const std::vector<std::vector<ecsBaseComponent*>>& components) 
{
	// Maintain list of reflectors, update with oldest within range
	// Technique will clear list when ready
	auto& reflectors = m_frameData.reflectorsToUpdate;
	auto& maxReflectors = m_maxReflectionCasters;
	const auto& clientCamera = m_engine.getModule_Graphics().getClientCamera();
	const auto& clientPosition = clientCamera->EyePosition;
	const auto& clientFarPlane = clientCamera->FarPlane;
	const auto clientTime = m_engine.GetSystemTime();
	if (const int availableRoom = (int)m_maxReflectionCasters - (int)m_frameData.reflectorsToUpdate.size()) {
		int cameraCount = 0;
		for (const auto& componentParam : components) {
			auto* reflectorComponent = static_cast<Reflector_Component*>(componentParam[0]);

			const auto tryToAddReflector = [&reflectors, &maxReflectors, &clientPosition, &clientFarPlane, &clientTime](const int& reflectorSpot, Camera* cb, float* updateTime) {
				const float linDist = glm::distance(clientPosition, cb->getFrustumCenter()) / clientFarPlane;
				const float importance_distance = 1.0f - (linDist * linDist);

				const float linTime = (clientTime - *updateTime) / 5.0f;
				const float importance_time = linTime * linTime;

				const float importance = importance_distance + importance_time * (1.0f - importance_distance);
				bool didAnything = false;
				{
					// Try to find the oldest components
					const auto reflectorCount = reflectors.size();
					for (int x = 0; x < reflectorCount; ++x) {
						auto& [oldImportance, oldTime, oldReflectorSpot, oldCamera] = reflectors[x];
						if ((oldReflectorSpot != -1 && importance > oldImportance)) {
							// Expand container by one
							reflectors.resize(reflectorCount + 1);
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
				}
				const auto reflectorCount = reflectors.size();
				if (!didAnything && reflectorCount < maxReflectors)
					reflectors.push_back({ importance, updateTime, reflectorSpot, cb });
				if (reflectorCount > maxReflectors)
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