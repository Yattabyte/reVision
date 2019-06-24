#pragma once
#ifndef POINTVISIBILITY_SYSTEM_H
#define POINTVISIBILITY_SYSTEM_H

#include "Modules/World/ECS/ecsSystem.h"
#include "Modules/World/ECS/components.h"
#include "Modules/Graphics/Logical/components.h"
#include "Modules/Graphics/Lighting/components.h"
#include "Utilities/GL/StaticBuffer.h"
#include "Utilities/GL/DynamicBuffer.h"
#include "glm/gtc/type_ptr.hpp"
#include <memory>


/***/
struct Point_Visibility {
	size_t visLightCount = 0ull;
	DynamicBuffer visLights;
	StaticBuffer indirectShape = StaticBuffer(sizeof(GLuint) * 4);
};

/***/
class PointVisibility_System : public BaseECSSystem {
public:
	// Public (de)Constructors
	/***/
	inline ~PointVisibility_System() = default;
	/***/
	inline PointVisibility_System(const std::shared_ptr<Point_Visibility> & visibility)
		: m_visibility(visibility) {
		addComponentType(Renderable_Component::ID, FLAG_REQUIRED);
		addComponentType(LightPoint_Component::ID, FLAG_REQUIRED);
	}


	// Public Interface Implementations
	inline virtual void updateComponents(const float & deltaTime, const std::vector<std::vector<BaseECSComponent*>> & components) override {
		std::vector<GLint> lightIndices;
		for each (const auto & componentParam in components) {
			Renderable_Component * renderableComponent = (Renderable_Component*)componentParam[0];
			LightPoint_Component * lightComponent = (LightPoint_Component*)componentParam[1];

			// Synchronize the component if it is visible
			if (renderableComponent->m_visible)
				lightIndices.push_back((GLuint)* lightComponent->m_lightIndex);
		}

		// Update camera buffers
		m_visibility->visLightCount = (GLsizei)lightIndices.size();
		m_visibility->visLights.write(0, sizeof(GLuint) * lightIndices.size(), lightIndices.data());
		m_visibility->indirectShape.write(sizeof(GLuint), sizeof(GLuint), &m_visibility->visLightCount); // update primCount (2nd param)
	}

private:
	// Private Attributes
	std::shared_ptr<Point_Visibility> m_visibility;
};

#endif // POINTVISIBILITY_SYSTEM_H