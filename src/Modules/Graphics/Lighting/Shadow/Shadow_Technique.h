#pragma once
#ifndef SHADOW_TECHNIQUE_H
#define SHADOW_TECHNIQUE_H

#include "Modules/Graphics/Common/Graphics_Technique.h"
#include "Modules/Graphics/Lighting/Shadow/ShadowData.h"
#include "Modules/Graphics/Lighting/Shadow/ShadowScheduler_System.h"
#include "Assets/Shader.h"
#include "Assets/Auto_Model.h"
#include "Engine.h"


/** A rendering technique responsible for rendering/updating shadow casters in the scene. */
class Shadow_Technique final : public Graphics_Technique {
public:
	// Public (de)Constructors
	/** Destructor. */
	inline ~Shadow_Technique() {
		// Update indicator
		*m_aliveIndicator = false;
	}
	/** Constructor. */
	inline Shadow_Technique(Engine* engine, const std::shared_ptr<std::vector<Camera*>>& cameras)
		: m_engine(engine), m_sceneCameras(cameras), Graphics_Technique(PRIMARY_LIGHTING) {
		m_frameData = std::make_shared<ShadowData>();
		m_auxilliarySystems.makeSystem<ShadowScheduler_System>(engine, m_frameData);

		// Preferences
		auto& preferences = m_engine->getPreferenceState();
		preferences.getOrSetValue(PreferenceState::C_SHADOW_SIZE, m_frameData->shadowSize);
		preferences.addCallback(PreferenceState::C_SHADOW_SIZE, m_aliveIndicator, [&](const float& f) {
			m_frameData->shadowSize = std::max(1.0f, f);
			m_frameData->shadowSizeRCP = 1.0f / m_frameData->shadowSize;
			});
		m_frameData->shadowSize = std::max(1.0f, m_frameData->shadowSize);
		m_frameData->shadowSizeRCP = 1.0f / m_frameData->shadowSize;
	}

	// Public Interface Implementations
	inline virtual void prepareForNextFrame(const float& deltaTime) override final {
		m_frameData->shadowsToUpdate.clear();
	}
	inline virtual void updateCache(const float& deltaTime, ecsWorld& world) override final {
		world.updateSystems(m_auxilliarySystems, deltaTime);
	}
	inline virtual void updatePass(const float& deltaTime) override final {
		// Render important shadows
		if (m_enabled)
			updateShadows(deltaTime);
	}


	// Public Methods
	inline std::shared_ptr<ShadowData> getShadowData() const {
		return m_frameData;
	}


private:
	// Private Methods
	/** Render all the geometry from each light.
	@param	deltaTime	the amount of time passed since last frame. */
	inline void updateShadows(const float& deltaTime) {
		auto clientTime = m_engine->getTime();
		if (m_frameData->shadowsToUpdate.size()) {
			// Prepare Viewport
			glViewport(0, 0, (GLsizei)m_frameData->shadowSize, (GLsizei)m_frameData->shadowSize);
			m_frameData->shadowFBO.bindForWriting();

			// Accumulate Perspective Data
			std::vector<std::pair<int, int>> perspectives;
			perspectives.reserve(m_frameData->shadowsToUpdate.size());
			for (auto& [importance, time, shadowSpot, camera] : m_frameData->shadowsToUpdate) {
				for (int y = 0; y < m_sceneCameras->size(); ++y)
					if (m_sceneCameras->at(y) == camera) {
						perspectives.push_back({ y, shadowSpot });
						break;
					}
				*time = clientTime;
			}

			// Perform shadow culling
			auto pipeline = m_engine->getModule_Graphics().getPipeline();
			pipeline->cullShadows(deltaTime, perspectives);
			for (auto& [importance, time, shadowSpot, camera] : m_frameData->shadowsToUpdate)
				m_frameData->shadowFBO.clear(shadowSpot, 1);

			// Render remaining shadows with populated buffers
			pipeline->renderShadows(deltaTime);
			m_frameData->shadowsToUpdate.clear();
		}
	}


	// Private Attributes
	Engine* m_engine = nullptr;
	std::shared_ptr<ShadowData> m_frameData;
	std::shared_ptr<std::vector<Camera*>> m_sceneCameras;
	ecsSystemList m_auxilliarySystems;
	std::shared_ptr<bool> m_aliveIndicator = std::make_shared<bool>(true);
};

#endif // SHADOW_TECHNIQUE_H