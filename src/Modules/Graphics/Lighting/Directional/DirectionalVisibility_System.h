#pragma once
#ifndef DIRECTIONALVISIBILITY_SYSTEM_H
#define DIRECTIONALVISIBILITY_SYSTEM_H

#include "Modules/World/ECS/ecsSystem.h"
#include "Modules/World/ECS/components.h"
#include "Modules/Graphics/Logical/components.h"
#include "Modules/Graphics/Lighting/components.h"
#include "Modules/Graphics/Lighting/Point/PointData.h"
#include "Engine.h"


/***/
class DirectionalVisibility_System : public BaseECSSystem {
public:
	// Public (de)Constructors
	/***/
	inline ~DirectionalVisibility_System() {
		// Update indicator
		m_aliveIndicator = false;
	}
	/***/
	inline DirectionalVisibility_System(Engine * engine, const std::shared_ptr<DirectionalData> & frameData, const std::shared_ptr<std::vector<std::shared_ptr<CameraBuffer>>> & cameras)
		: m_frameData(frameData), m_cameras(cameras) {
		addComponentType(Renderable_Component::ID, FLAG_REQUIRED);
		addComponentType(LightDirectional_Component::ID, FLAG_REQUIRED);
		addComponentType(Shadow_Component::ID, FLAG_REQUIRED);
		addComponentType(CameraArray_Component::ID, FLAG_REQUIRED);

		auto & preferences = engine->getPreferenceState();
		preferences.getOrSetValue(PreferenceState::C_RH_BOUNCE_SIZE, m_bounceSize);
		preferences.addCallback(PreferenceState::C_RH_BOUNCE_SIZE, m_aliveIndicator, [&](const float &f) { m_bounceSize = (GLuint)f; });
	}


	// Public Interface Implementations
	inline virtual void updateComponents(const float & deltaTime, const std::vector<std::vector<BaseECSComponent*>> & components) override {
		// Link together the dimensions of view info and light buffers to that of the viewport vectors
		m_frameData->viewInfo.resize(m_cameras->size());

		// Compile results PER viewport
		for (int x = 0; x < m_frameData->viewInfo.size(); ++x) {
			auto & viewInfo = m_frameData->viewInfo[x];

			std::vector<GLint> lightIndices;
			int index = 0;
			int shadowCount = 0;
			for each (const auto & componentParam in components) {
				Renderable_Component * renderableComponent = (Renderable_Component*)componentParam[0];
				LightDirectional_Component * lightComponent = (LightDirectional_Component*)componentParam[1];
				Shadow_Component * shadowComponent = (Shadow_Component*)componentParam[2];
				CameraArray_Component * cameraComponent = (CameraArray_Component*)componentParam[3];

				// Render lights and shadows for all visible directional lights
				if (renderableComponent->m_visible[x])
					lightIndices.push_back((GLuint)index);

				if (renderableComponent->m_visibleAtAll)
					shadowCount++;
				index++;
			}

			// Update camera buffers
			viewInfo.visLights.beginWriting();
			viewInfo.visLightCount = (GLsizei)lightIndices.size();
			viewInfo.visShadowCount = (GLsizei)shadowCount;
			viewInfo.visLights.write(0, sizeof(GLuint) * lightIndices.size(), lightIndices.data());
			const GLuint dataShape[] = { (GLuint)m_frameData->shapeVertexCount, (GLuint)viewInfo.visLightCount,0,0 };
			const GLuint dataBounce[] = { (GLuint)m_frameData->shapeVertexCount, (GLuint)(viewInfo.visShadowCount * m_bounceSize),0,0 };
			viewInfo.indirectShape.write(0, sizeof(GLuint) * 4, &dataShape);
			viewInfo.indirectBounce.write(0, sizeof(GLuint) * 4, &dataBounce);
		}
	}


private:
	// Private Attributes
	GLuint m_bounceSize = 16u;
	std::shared_ptr<DirectionalData> m_frameData;
	std::shared_ptr<std::vector<std::shared_ptr<CameraBuffer>>> m_cameras;
	std::shared_ptr<bool> m_aliveIndicator = std::make_shared<bool>(true);
};

#endif // DIRECTIONALVISIBILITY_SYSTEM_H