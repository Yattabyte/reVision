#pragma once
#ifndef REFLECTORVISIBILITY_SYSTEM_H
#define REFLECTORVISIBILITY_SYSTEM_H

#include "Modules/World/ECS/ecsSystem.h"
#include "Modules/World/ECS/components.h"
#include "Modules/Graphics/Logical/components.h"
#include "Modules/Graphics/Lighting/components.h"
#include "Utilities/GL/StaticBuffer.h"
#include "Utilities/GL/DynamicBuffer.h"
#include <memory>


/***/
struct Reflector_Visibility {
	size_t visLightCount = 0ull;
	DynamicBuffer visLights;
	StaticBuffer
		indirectCube = StaticBuffer(sizeof(GLuint) * 4), 
		indirectQuad = StaticBuffer(sizeof(GLuint) * 4), 
		indirectQuad6Faces = StaticBuffer(sizeof(GLuint) * 4);
};

/***/
class ReflectorVisibility_System : public BaseECSSystem {
public:
	// Public (de)Constructors
	/***/
	inline ~ReflectorVisibility_System() = default;
	/***/
	inline ReflectorVisibility_System(const std::shared_ptr<Reflector_Visibility> & visibility)
		: m_visibility(visibility) {
		addComponentType(Renderable_Component::ID, FLAG_REQUIRED);
		addComponentType(Reflector_Component::ID, FLAG_REQUIRED);
	}


	// Public Interface Implementations
	inline virtual void updateComponents(const float & deltaTime, const std::vector<std::vector<BaseECSComponent*>> & components) override {
		std::vector<GLint> lightIndices;
		for each (const auto & componentParam in components) {
			Renderable_Component * renderableComponent = (Renderable_Component*)componentParam[0];
			Reflector_Component * reflectorComponent = (Reflector_Component*)componentParam[1];

			// Synchronize the component if it is visible
			if (renderableComponent->m_visible)
				lightIndices.push_back((GLuint)* reflectorComponent->m_reflectorIndex);
		}

		// Update camera buffers
		m_visibility->visLightCount = (GLsizei)lightIndices.size();
		m_visibility->visLights.write(0, sizeof(GLuint) * lightIndices.size(), lightIndices.data());
		m_visibility->indirectCube.write(sizeof(GLuint), sizeof(GLuint), &m_visibility->visLightCount); // update primCount (2nd param)
	}

private:
	// Private Attributes
	std::shared_ptr<Reflector_Visibility> m_visibility;
};

#endif // REFLECTORVISIBILITY_SYSTEM_H