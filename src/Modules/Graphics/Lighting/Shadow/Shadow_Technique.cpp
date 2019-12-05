#include "Modules/Graphics/Lighting/Shadow/Shadow_Technique.h"


Shadow_Technique::~Shadow_Technique() noexcept 
{
	// Update indicator
	*m_aliveIndicator = false;
}

Shadow_Technique::Shadow_Technique(Engine& engine, std::vector<Camera*>& sceneCameras) noexcept :
	Graphics_Technique(Technique_Category::PRIMARY_LIGHTING),
	m_engine(engine),
	m_sceneCameras(sceneCameras)
{
	m_auxilliarySystems.makeSystem<ShadowScheduler_System>(engine, m_frameData);

	// Preferences
	auto& preferences = engine.getPreferenceState();
	preferences.getOrSetValue(PreferenceState::Preference::C_SHADOW_SIZE, m_frameData.shadowSize);
	preferences.addCallback(PreferenceState::Preference::C_SHADOW_SIZE, m_aliveIndicator, [&](const float& f) {
		m_frameData.shadowSize = std::max(1.0f, f);
		m_frameData.shadowSizeRCP = 1.0f / m_frameData.shadowSize;
		});
	m_frameData.shadowSize = std::max(1.0f, m_frameData.shadowSize);
	m_frameData.shadowSizeRCP = 1.0f / m_frameData.shadowSize;
}

void Shadow_Technique::clearCache(const float& deltaTime) noexcept 
{
	m_frameData.shadowsToUpdate.clear();
}

void Shadow_Technique::updateCache(const float& deltaTime, ecsWorld& world) noexcept 
{
	world.updateSystems(m_auxilliarySystems, deltaTime);
}

void Shadow_Technique::updatePass(const float& deltaTime) noexcept 
{
	// Render important shadows
	if (m_enabled)
		updateShadows(deltaTime);
}

ShadowData& Shadow_Technique::getShadowData() noexcept 
{
	return m_frameData;
}

void Shadow_Technique::updateShadows(const float& deltaTime) noexcept 
{
	auto clientTime = m_engine.getTime();
	if (m_frameData.shadowsToUpdate.size()) {
		// Prepare Viewport
		glViewport(0, 0, (GLsizei)m_frameData.shadowSize, (GLsizei)m_frameData.shadowSize);
		m_frameData.shadowFBO.bindForWriting();

		// Accumulate Perspective Data
		std::vector<std::pair<int, int>> perspectives;
		perspectives.reserve(m_frameData.shadowsToUpdate.size());
		for (auto& [importance, time, shadowSpot, camera] : m_frameData.shadowsToUpdate) {
			for (int y = 0; y < m_sceneCameras.size(); ++y)
				if (m_sceneCameras.at(y) == camera) {
					perspectives.push_back({ y, shadowSpot });
					break;
				}
			*time = clientTime;
		}

		// Perform shadow culling
		auto pipeline = m_engine.getModule_Graphics().getPipeline();
		pipeline->cullShadows(deltaTime, perspectives);
		for (auto& [importance, time, shadowSpot, camera] : m_frameData.shadowsToUpdate)
			m_frameData.shadowFBO.clear(shadowSpot, 1);

		// Render remaining shadows with populated buffers
		pipeline->renderShadows(deltaTime);
		m_frameData.shadowsToUpdate.clear();
	}
}