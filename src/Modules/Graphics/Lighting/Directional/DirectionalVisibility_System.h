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
	size_t visLightCount = 0ull, visShadowCount = 0ull;
	DynamicBuffer visLights;
	StaticBuffer indirectShape = StaticBuffer(sizeof(GLuint) * 4), indirectBounce = StaticBuffer(sizeof(GLuint) * 4);
	std::vector<LightDirectional_Component*> shadowsToUpdate;
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
	inline DirectionalVisibility_System(Engine * engine, const std::shared_ptr<Directional_Visibility> & visibility)
		: m_visibility(visibility) {
		addComponentType(Renderable_Component::ID, FLAG_REQUIRED);
		addComponentType(LightDirectional_Component::ID, FLAG_REQUIRED);


		auto & preferences = engine->getPreferenceState();
		preferences.getOrSetValue(PreferenceState::C_RH_BOUNCE_SIZE, m_bounceSize);
		preferences.addCallback(PreferenceState::C_RH_BOUNCE_SIZE, m_aliveIndicator, [&](const float &f) { m_bounceSize = (GLuint)f; });
	}


	// Public Interface Implementations
	inline virtual void updateComponents(const float & deltaTime, const std::vector<std::vector<BaseECSComponent*>> & components) override {
		std::vector<GLint> lightIndices;
		m_visibility->shadowsToUpdate.clear();
		for each (const auto & componentParam in components) {
			Renderable_Component * renderableComponent = (Renderable_Component*)componentParam[0];
			LightDirectional_Component * lightComponent = (LightDirectional_Component*)componentParam[1];

			// Synchronize the component if it is visible
			if (renderableComponent->m_visible) {
				lightIndices.push_back((GLuint)* lightComponent->m_lightIndex);
				if (lightComponent->m_hasShadow) 
					m_visibility->shadowsToUpdate.push_back(lightComponent);				
			}
		}

		// Update camera buffers
		m_visibility->visLightCount = (GLsizei)lightIndices.size();
		m_visibility->visShadowCount = (GLsizei)m_visibility->shadowsToUpdate.size();
		const GLuint bounceInstanceCount = GLuint(m_visibility->visShadowCount * m_bounceSize);
		m_visibility->visLights.write(0, sizeof(GLuint) * lightIndices.size(), lightIndices.data());
		m_visibility->indirectShape.write(sizeof(GLuint), sizeof(GLuint), &m_visibility->visLightCount); // update primCount (2nd param)
		m_visibility->indirectBounce.write(sizeof(GLuint), sizeof(GLuint), &bounceInstanceCount);
	}

private:
	// Private Attributes
	GLuint m_bounceSize = 16u;
	std::shared_ptr<Directional_Visibility> m_visibility;
	std::shared_ptr<bool> m_aliveIndicator = std::make_shared<bool>(true);
};

#endif // DIRECTIONALVISIBILITY_SYSTEM_H