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
	struct ViewInfo {
		size_t visLightCount = 0ull;
		DynamicBuffer visLights;
		StaticBuffer indirectCube = StaticBuffer(sizeof(GLuint) * 4);
	};
	std::vector<ViewInfo> viewInfo;
	size_t shapeVertexCount = 0ull;
};

/***/
class ReflectorVisibility_System : public BaseECSSystem {
public:
	// Public (de)Constructors
	/***/
	inline ~ReflectorVisibility_System() = default;
	/***/
	inline ReflectorVisibility_System(const std::shared_ptr<Reflector_Visibility> & visibility, const std::shared_ptr<std::vector<Viewport*>> & viewports)
		: m_visibility(visibility), m_viewports(viewports) {
		addComponentType(Renderable_Component::ID, FLAG_REQUIRED);
		addComponentType(Reflector_Component::ID, FLAG_REQUIRED);
	}


	// Public Interface Implementations
	inline virtual void updateComponents(const float & deltaTime, const std::vector<std::vector<BaseECSComponent*>> & components) override {
		// Link together the dimensions of view info and viewport vectors
		m_visibility->viewInfo.resize(m_viewports->size());
		// Compile results PER viewport
		for (int x = 0; x < m_viewports->size(); ++x) {
			auto * viewport = m_viewports->at(x);
			auto & viewInfo = m_visibility->viewInfo[x];

			std::vector<GLint> lightIndices;
			for each (const auto & componentParam in components) {
				Renderable_Component * renderableComponent = (Renderable_Component*)componentParam[0];
				Reflector_Component * reflectorComponent = (Reflector_Component*)componentParam[1];

				// Synchronize the component if it is visible
				if (renderableComponent->m_visible[x]) 
					lightIndices.push_back((GLuint)* reflectorComponent->m_reflectorIndex);				
			}

			// Update camera buffers
			viewInfo.visLights.beginWriting();
			viewInfo.visLightCount = (GLsizei)lightIndices.size();
			viewInfo.visLights.write(0, sizeof(GLuint) * lightIndices.size(), lightIndices.data());
			// count, primCount, first, reserved
			const GLuint data[] = { (GLuint)m_visibility->shapeVertexCount, (GLuint)viewInfo.visLightCount,0,0 };
			viewInfo.indirectCube.write(0, sizeof(GLuint) * 4, &data);		
		}
	}


private:
	// Private Attributes
	std::shared_ptr<Reflector_Visibility> m_visibility;
	std::shared_ptr<std::vector<Viewport*>> m_viewports;
};

#endif // REFLECTORVISIBILITY_SYSTEM_H