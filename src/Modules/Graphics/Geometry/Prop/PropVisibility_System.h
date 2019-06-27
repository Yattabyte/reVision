#pragma once
#ifndef PROPVISIBILITY_SYSTEM_H
#define PROPVISIBILITY_SYSTEM_H

#include "Modules/World/ECS/ecsSystem.h"
#include "Modules/World/ECS/components.h"
#include "Modules/Graphics/Logical/components.h"
#include "Modules/Graphics/Geometry/components.h"
#include "Utilities/GL/DynamicBuffer.h"
#include "glm/gtc/type_ptr.hpp"
#include <memory>


/***/
struct Prop_Visibility {
	struct ViewInfo {
		GLsizei visProps = 0;
		DynamicBuffer bufferPropIndex, bufferCulling, bufferRender, bufferSkeletonIndex;
	};
	std::vector<ViewInfo> viewInfo;
};

/***/
class PropVisibility_System : public BaseECSSystem {
public:
	// Public (de)Constructors
	/***/
	inline ~PropVisibility_System() = default;
	/***/
	inline PropVisibility_System(const std::shared_ptr<Prop_Visibility> & visibility, const std::shared_ptr<std::vector<Viewport*>> & viewports)
		: m_visibility(visibility), m_viewports(viewports) {
		addComponentType(Renderable_Component::ID, FLAG_REQUIRED);
		addComponentType(Prop_Component::ID, FLAG_REQUIRED);
		addComponentType(Skeleton_Component::ID, FLAG_OPTIONAL);
		addComponentType(BoundingSphere_Component::ID, FLAG_OPTIONAL);
	}


	// Public Interface Implementations
	inline virtual void updateComponents(const float & deltaTime, const std::vector<std::vector<BaseECSComponent*>> & components) override {
		// Link together the dimensions of view info and viewport vectors
		m_visibility->viewInfo.resize(m_viewports->size());
		// Compile results PER viewport
		for (int x = 0; x < m_viewports->size(); ++x) {
			auto * viewport = m_viewports->at(x);
			auto & viewInfo = m_visibility->viewInfo[x];

			std::vector<glm::ivec4> cullingDrawData, renderingDrawData;
			std::vector<GLuint> visibleIndices;
			std::vector<int> skeletonData;
			for each (const auto & componentParam in components) {
				Renderable_Component * renderableComponent = (Renderable_Component*)componentParam[0];
				Prop_Component * propComponent = (Prop_Component*)componentParam[1];
				Skeleton_Component * skeletonComponent = (Skeleton_Component*)componentParam[2];
				BoundingSphere_Component * bsphereComponent = (BoundingSphere_Component*)componentParam[3];
				const auto & offset = propComponent->m_model->m_offset;
				const auto & count = propComponent->m_model->m_count;
				const auto & index = propComponent->m_modelBufferIndex;

				if (renderableComponent->m_visible[x]) {
					visibleIndices.push_back((GLuint)*index);
					skeletonData.push_back(skeletonComponent ? (GLint)*skeletonComponent->m_skeleBufferIndex : -1); // get skeleton ID if this entity has one
					// Flag for occlusion culling if mesh complexity is high enough and if viewer is NOT within BSphere
					if ((count >= 100) && bsphereComponent && bsphereComponent->m_cameraCollision == BoundingSphere_Component::OUTSIDE) {
						// Allow occlusion culling	
						cullingDrawData.push_back(glm::ivec4(36, 1, 0, 1));
						renderingDrawData.push_back(glm::ivec4(count, 0, offset, 1));
					}
					else {
						// Skip occlusion culling		
						cullingDrawData.push_back(glm::ivec4(36, 0, 0, 1));
						renderingDrawData.push_back(glm::ivec4(count, 1, offset, 1));
					}
				}
			}

			// Update camera buffers
			viewInfo.bufferPropIndex.beginWriting();
			viewInfo.bufferCulling.beginWriting();
			viewInfo.bufferRender.beginWriting();
			viewInfo.bufferSkeletonIndex.beginWriting();
			viewInfo.visProps = (GLsizei)visibleIndices.size();
			viewInfo.bufferPropIndex.write(0, sizeof(GLuint) * visibleIndices.size(), visibleIndices.data());
			viewInfo.bufferCulling.write(0, sizeof(glm::ivec4) * cullingDrawData.size(), cullingDrawData.data());
			viewInfo.bufferRender.write(0, sizeof(glm::ivec4) * renderingDrawData.size(), renderingDrawData.data());
			viewInfo.bufferSkeletonIndex.write(0, sizeof(int) * skeletonData.size(), skeletonData.data());
		}
	}


private:
	// Private Attributes
	std::shared_ptr<Prop_Visibility> m_visibility;
	std::shared_ptr<std::vector<Viewport*>> m_viewports;
};

#endif // PROPVISIBILITY_SYSTEM_H