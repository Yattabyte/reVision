#pragma once
#ifndef PROPVISIBILITY_SYSTEM_H
#define PROPVISIBILITY_SYSTEM_H

#include "Modules/World/ECS/ecsSystem.h"
#include "Modules/World/ECS/components.h"
#include "Modules/Graphics/Logical/components.h"
#include "Modules/Graphics/Geometry/components.h"
#include "Modules/Graphics/Geometry/Prop/PropData.h"


/***/
class PropVisibility_System : public BaseECSSystem {
public:
	// Public (de)Constructors
	/***/
	inline ~PropVisibility_System() = default;
	/***/
	inline PropVisibility_System(const std::shared_ptr<PropData> & frameData, const std::shared_ptr<std::vector<std::shared_ptr<CameraBuffer>>> & cameras)
		: m_frameData(frameData), m_cameras(cameras) {
		addComponentType(Renderable_Component::ID, FLAG_REQUIRED);
		addComponentType(Prop_Component::ID, FLAG_REQUIRED);
		addComponentType(Skeleton_Component::ID, FLAG_OPTIONAL);
		addComponentType(BoundingSphere_Component::ID, FLAG_OPTIONAL);
	}


	// Public Interface Implementations
	inline virtual void updateComponents(const float & deltaTime, const std::vector<std::vector<BaseECSComponent*>> & components) override {
		// Link together the dimensions of view info and viewport vectors
		m_frameData->viewInfo.resize(m_cameras->size());

		// Compile results PER viewport
		for (int x = 0; x < m_frameData->viewInfo.size(); ++x) {
			auto & viewInfo = m_frameData->viewInfo[x];

			std::vector<glm::ivec4> cullingDrawData, renderingDrawData;
			std::vector<GLuint> visibleIndices;
			std::vector<int> skeletonData;
			int index = 0;
			for each (const auto & componentParam in components) {
				Renderable_Component * renderableComponent = (Renderable_Component*)componentParam[0];
				Prop_Component * propComponent = (Prop_Component*)componentParam[1];
				Skeleton_Component * skeletonComponent = (Skeleton_Component*)componentParam[2];
				BoundingSphere_Component * bsphereComponent = (BoundingSphere_Component*)componentParam[3];
				const auto & offset = propComponent->m_offset;
				const auto & count = propComponent->m_count;

				if (renderableComponent->m_visible[x]) {
					const auto & isStatic = propComponent->m_static;
					visibleIndices.push_back((GLuint)index);
					
					skeletonData.push_back(skeletonComponent ? index : -1); // get skeleton ID if this entity has one
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
				index++;
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
	std::shared_ptr<PropData> m_frameData;
	std::shared_ptr<std::vector<std::shared_ptr<CameraBuffer>>> m_cameras;
};

#endif // PROPVISIBILITY_SYSTEM_H