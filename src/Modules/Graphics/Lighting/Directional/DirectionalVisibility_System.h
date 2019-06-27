#pragma once
#ifndef DIRECTIONALVISIBILITY_SYSTEM_H
#define DIRECTIONALVISIBILITY_SYSTEM_H

#include "Modules/World/ECS/ecsSystem.h"
#include "Modules/World/ECS/components.h"
#include "Modules/Graphics/Logical/components.h"
#include "Modules/Graphics/Lighting/components.h"
#include "Utilities/GL/StaticBuffer.h"
#include "Utilities/GL/DynamicBuffer.h"
#include "Engine.h"
#include "glm/gtc/type_ptr.hpp"
#include <memory>
#include <vector>


/***/
struct Directional_Visibility {
	struct ViewInfo {
		// Directional shadows updated PER viewport, because they are CSM from perspective
		size_t visLightCount = 0ull, visShadowCount = 0ull;
		DynamicBuffer visLights;
		StaticBuffer indirectShape = StaticBuffer(sizeof(GLuint) * 4), indirectBounce = StaticBuffer(sizeof(GLuint) * 4);
		std::vector<LightDirectional_Component*> shadowsToUpdate;
	};
	std::vector<ViewInfo> viewInfo;
	size_t shapeVertexCount = 0ull;
};

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
	inline DirectionalVisibility_System(Engine * engine, const std::shared_ptr<Directional_Visibility> & visibility, const std::shared_ptr<std::vector<Viewport*>> & viewports)
		: m_visibility(visibility), m_viewports(viewports) {
		addComponentType(Renderable_Component::ID, FLAG_REQUIRED);
		addComponentType(LightDirectional_Component::ID, FLAG_REQUIRED);
		
		auto & preferences = engine->getPreferenceState();
		preferences.getOrSetValue(PreferenceState::C_RH_BOUNCE_SIZE, m_bounceSize);
		preferences.addCallback(PreferenceState::C_RH_BOUNCE_SIZE, m_aliveIndicator, [&](const float &f) { m_bounceSize = (GLuint)f; });
	}


	// Public Interface Implementations
	inline virtual void updateComponents(const float & deltaTime, const std::vector<std::vector<BaseECSComponent*>> & components) override {
		// Link together the dimensions of view info and viewport vectors
		auto oldSize = m_visibility->viewInfo.size();
		m_visibility->viewInfo.resize(m_viewports->size());
		for (int x = oldSize; x < m_viewports->size(); ++x) {
			// count, primCount, first, reserved
			const GLuint data[] = { (GLuint)m_visibility->shapeVertexCount,0,0,0 };
			m_visibility->viewInfo[x].indirectShape.write(0, sizeof(GLuint) * 4, &data);
			m_visibility->viewInfo[x].indirectBounce.write(0, sizeof(GLuint) * 4, &data);
		}

		// Compile results PER viewport
		for (int x = 0; x < m_viewports->size(); ++x) {
			auto * viewport = m_viewports->at(x);
			auto & viewInfo = m_visibility->viewInfo[x];

			std::vector<GLint> lightIndices;
			viewInfo.shadowsToUpdate.clear();
			for each (const auto & componentParam in components) {
				Renderable_Component * renderableComponent = (Renderable_Component*)componentParam[0];
				LightDirectional_Component * lightComponent = (LightDirectional_Component*)componentParam[1];

				// Render lights and shadows for all visible directional lights
				if (renderableComponent->m_visible[x]) {
					lightIndices.push_back((GLuint)* lightComponent->m_lightIndex);
					if (lightComponent->m_hasShadow)
						viewInfo.shadowsToUpdate.push_back(lightComponent);
				}
			}

			// Update camera buffers
			viewInfo.visLights.beginWriting();
			viewInfo.visLightCount = (GLsizei)lightIndices.size();
			viewInfo.visShadowCount = (GLsizei)viewInfo.shadowsToUpdate.size();
			const GLuint bounceInstanceCount = GLuint(viewInfo.visShadowCount * m_bounceSize);
			viewInfo.visLights.write(0, sizeof(GLuint) * lightIndices.size(), lightIndices.data());
			viewInfo.indirectShape.write(sizeof(GLuint), sizeof(GLuint), &viewInfo.visLightCount); // update primCount (2nd param)
			viewInfo.indirectBounce.write(sizeof(GLuint), sizeof(GLuint), &bounceInstanceCount);
		}
	}

private:
	// Private Attributes
	GLuint m_bounceSize = 16u;
	std::shared_ptr<Directional_Visibility> m_visibility;
	std::shared_ptr<std::vector<Viewport*>> m_viewports;
	std::shared_ptr<bool> m_aliveIndicator = std::make_shared<bool>(true);
};

#endif // DIRECTIONALVISIBILITY_SYSTEM_H