#pragma once
#ifndef SPOTVISIBILITY_SYSTEM_H
#define SPOTVISIBILITY_SYSTEM_H

#include "Modules/World/ECS/ecsSystem.h"
#include "Modules/World/ECS/components.h"
#include "Modules/Graphics/Logical/components.h"
#include "Modules/Graphics/Lighting/components.h"
#include "Utilities/GL/StaticBuffer.h"
#include "Utilities/GL/DynamicBuffer.h"
#include <memory>


/***/
struct Spot_Visibility {
	struct ViewInfo {
		size_t visLightCount = 0ull;
		DynamicBuffer visLights;
		StaticBuffer indirectShape = StaticBuffer(sizeof(GLuint) * 4);
	};
	std::vector<ViewInfo> viewInfo;
	size_t shapeVertexCount = 0ull;
};

/***/
class SpotVisibility_System : public BaseECSSystem {
public:
	// Public (de)Constructors
	/***/
	inline ~SpotVisibility_System() = default;
	/***/
	inline SpotVisibility_System(const std::shared_ptr<Spot_Visibility> & visibility, const std::shared_ptr<std::vector<Viewport*>> & viewports)
		: m_visibility(visibility), m_viewports(viewports) {
		addComponentType(Renderable_Component::ID, FLAG_REQUIRED);
		addComponentType(LightSpot_Component::ID, FLAG_REQUIRED);
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
				LightSpot_Component * lightComponent = (LightSpot_Component*)componentParam[1];

				// Synchronize the component if it is visible
				if (renderableComponent->m_visible[x])
					lightIndices.push_back((GLuint)* lightComponent->m_lightIndex);
			}

			// Update camera buffers
			viewInfo.visLights.beginWriting();
			viewInfo.visLightCount = (GLsizei)lightIndices.size();
			viewInfo.visLights.write(0, sizeof(GLuint) * lightIndices.size(), lightIndices.data());
			// count, primCount, first, reserved
			const GLuint data[] = { (GLuint)m_visibility->shapeVertexCount, (GLuint)viewInfo.visLightCount,0,0 };
			viewInfo.indirectShape.write(0, sizeof(GLuint) * 4, &data);
		}
	}


private:
	// Private Attributes
	std::shared_ptr<Spot_Visibility> m_visibility;
	std::shared_ptr<std::vector<Viewport*>> m_viewports;
};

#endif // SPOTVISIBILITY_SYSTEM_H