#pragma once
#ifndef PROPVISIBILITY_SYSTEM_H
#define PROPVISIBILITY_SYSTEM_H

#include "Modules/ECS/ecsSystem.h"
#include "Modules/ECS/component_types.h"
#include "Modules/Graphics/Geometry/Prop/PropData.h"


/** An ECS system responsible for populating render lists PER active perspective in a given frame, for all prop related entities. */
class PropVisibility_System final : public ecsBaseSystem {
public:
	// Public (De)Constructors
	/** Destroy this system. */
	inline ~PropVisibility_System() = default;
	/** Construct this system.
	@param	frameData	shared pointer of common data that changes frame-to-frame. */
	inline PropVisibility_System(const std::shared_ptr<PropData>& frameData, const std::shared_ptr<std::vector<Camera*>>& cameras) noexcept :
		m_frameData(frameData),
		m_cameras(cameras)
	{
		addComponentType(Prop_Component::Runtime_ID, RequirementsFlag::FLAG_REQUIRED);
		addComponentType(Skeleton_Component::Runtime_ID, RequirementsFlag::FLAG_OPTIONAL);
		addComponentType(BoundingBox_Component::Runtime_ID, RequirementsFlag::FLAG_OPTIONAL);
	}


	// Public Interface Implementations
	inline virtual void updateComponents(const float& deltaTime, const std::vector<std::vector<ecsBaseComponent*>>& components) noexcept override final {
		// Compile results PER viewport
		for (int x = 0; x < m_frameData->viewInfo.size(); ++x) {
			auto& viewInfo = m_frameData->viewInfo[x];
			viewInfo.cullingDrawData.clear();
			viewInfo.renderingDrawData.clear();
			viewInfo.visibleIndices.clear();
			viewInfo.skeletonData.clear();
			int index = 0;
			for (const auto& componentParam : components) {
				auto* propComponent = static_cast<Prop_Component*>(componentParam[0]);
				auto* skeletonComponent = static_cast<Skeleton_Component*>(componentParam[1]);
				auto* bboxComponent = static_cast<BoundingBox_Component*>(componentParam[2]);
				const auto& offset = propComponent->m_offset;
				const auto& count = propComponent->m_count;

				if (count && propComponent->m_uploadModel && propComponent->m_uploadMaterial) {
					viewInfo.visibleIndices.push_back((GLuint)index);

					viewInfo.skeletonData.push_back(skeletonComponent ? index : -1); // get skeleton ID if this entity has one
					// Flag for occlusion culling if mesh complexity is high enough and if viewer is NOT within BSphere
					if ((count >= 100) && bboxComponent && bboxComponent->m_cameraCollision == BoundingBox_Component::CameraCollision::OUTSIDE) {
						// Allow occlusion culling
						viewInfo.cullingDrawData.push_back(glm::ivec4(36, 1, 0, 1));
						viewInfo.renderingDrawData.push_back(glm::ivec4(count, 0, offset, 1));
					}
					else {
						// Skip occlusion culling
						viewInfo.cullingDrawData.push_back(glm::ivec4(36, 0, 0, 1));
						viewInfo.renderingDrawData.push_back(glm::ivec4(count, 1, offset, 1));
					}
				}
				index++;
			}
		}
	}


private:
	// Private Attributes
	std::shared_ptr<PropData> m_frameData;
	std::shared_ptr<std::vector<Camera*>> m_cameras;
};

#endif // PROPVISIBILITY_SYSTEM_H
