#pragma once
#ifndef SHADOW_TECHNIQUE_H
#define SHADOW_TECHNIQUE_H

#include "Modules/Graphics/Common/Graphics_Technique.h"
#include "Modules/Graphics/Lighting/components.h"
#include "Modules/Graphics/Lighting/Shadow/ShadowData.h"
#include "Modules/Graphics/Lighting/Shadow/ShadowScheduler_System.h"
#include "Assets/Shader.h"
#include "Assets/Primitive.h"
#include "Engine.h"


/***/
class Shadow_Technique : public Graphics_Technique {
public:
	// Public (de)Constructors
	/** Destructor. */
	inline ~Shadow_Technique() {
		// Update indicator
		m_aliveIndicator = false;
	}
	/** Constructor. */
	inline Shadow_Technique(Engine * engine, ECSSystemList & auxilliarySystems)
		: m_engine(engine), Graphics_Technique(PRIMARY_LIGHTING) {
		m_frameData = std::make_shared<ShadowData>();
		auxilliarySystems.addSystem(new ShadowScheduler_System(m_engine, m_frameData));

		// Preferences
		auto & preferences = m_engine->getPreferenceState();
		preferences.getOrSetValue(PreferenceState::C_SHADOW_SIZE, m_frameData->shadowSize);
		preferences.addCallback(PreferenceState::C_SHADOW_SIZE, m_aliveIndicator, [&](const float &f) {
			m_frameData->shadowSize = std::max(1.0f, f);
			m_frameData->shadowSizeRCP = 1.0f / m_frameData->shadowSize;
		});
		m_frameData->shadowSize = std::max(1.0f, m_frameData->shadowSize);
		m_frameData->shadowSizeRCP = 1.0f / m_frameData->shadowSize;

		// Clear state on world-unloaded
		m_engine->getModule_World().addLevelListener(m_aliveIndicator, [&](const World_Module::WorldState & state) {
			if (state == World_Module::unloaded)
				clear();
		});
	}


	// Public Interface Implementations
	inline virtual void updateTechnique(const float & deltaTime) override {
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
	inline void updateShadows(const float & deltaTime) {
		if (m_frameData->shadowsToUpdate.size()) {
			// Prepare Viewport
			glViewport(0, 0, m_frameData->shadowSize, m_frameData->shadowSize);
			m_frameData->shadowFBO.bindForWriting();

			// Accumulate Perspective Data
			std::vector<std::pair<CameraBuffer::CamStruct*, int>> perspectives;
			perspectives.reserve(m_frameData->shadowsToUpdate.size());
			for (auto &[time, shadowSpot, cameras] : m_frameData->shadowsToUpdate) {
				m_frameData->shadowFBO.clear(shadowSpot, cameras.size());
				for (int x = 0; x < cameras.size(); ++x)
					perspectives.push_back({ cameras[x], shadowSpot + x });				
				*time += deltaTime;
			}
			
			// Perform shadow culling
			m_engine->getModule_Graphics().cullShadows(deltaTime, perspectives);
			for (auto &[time, shadowSpot, cameras] : m_frameData->shadowsToUpdate)
				m_frameData->shadowFBO.clear(shadowSpot, cameras.size());

			// Render remaining shadows with populated buffers
			m_engine->getModule_Graphics().renderShadows(deltaTime);
			m_frameData->shadowsToUpdate.clear();
		}
	}
	/** Clear out the lights and shadows queued up for rendering. */
	inline void clear() {
		m_frameData->shadowsToUpdate.clear();
	}


	// Private Attributes
	Engine * m_engine = nullptr;
	std::shared_ptr<ShadowData> m_frameData;
	std::shared_ptr<bool> m_aliveIndicator = std::make_shared<bool>(true);
};

#endif // SHADOW_TECHNIQUE_H