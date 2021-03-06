#include "Modules/Graphics/Geometry/Prop/PropVisibility_System.h"
#include "Modules/Graphics/Geometry/Prop/PropData.h"
#include "Modules/ECS/component_types.h"


PropVisibility_System::PropVisibility_System(PropData& frameData) :
	m_frameData(frameData)
{
	addComponentType(Prop_Component::Runtime_ID, RequirementsFlag::FLAG_REQUIRED);
	addComponentType(Skeleton_Component::Runtime_ID, RequirementsFlag::FLAG_OPTIONAL);
	addComponentType(BoundingBox_Component::Runtime_ID, RequirementsFlag::FLAG_OPTIONAL);
}

void PropVisibility_System::updateComponents(const float& /*deltaTime*/, const std::vector<std::vector<ecsBaseComponent*>>& components)
{
	// Compile results PER viewport
	for (auto& viewInfo : m_frameData.viewInfo) {
		viewInfo.cullingDrawData.clear();
		viewInfo.renderingDrawData.clear();
		viewInfo.visibleIndices.clear();
		viewInfo.skeletonData.clear();
		int index = 0;
		for (const auto& componentParam : components) {
			const auto* propComponent = static_cast<Prop_Component*>(componentParam[0]);
			const auto* skeletonComponent = dynamic_cast<Skeleton_Component*>(componentParam[1]);
			const auto* bboxComponent = dynamic_cast<BoundingBox_Component*>(componentParam[2]);
			const auto& offset = propComponent->m_offset;
			const auto& count = propComponent->m_count;

			if ((count != 0U) && propComponent->m_uploadModel && propComponent->m_uploadMaterial) {
				viewInfo.visibleIndices.push_back(static_cast<GLuint>(index));
				viewInfo.skeletonData.push_back(skeletonComponent != nullptr ? index : -1); // get skeleton ID if this entity has one

				// Flag for occlusion culling if mesh complexity is high enough and if viewer is NOT within BSphere
				if ((count >= 100) && (bboxComponent != nullptr) && bboxComponent->m_cameraCollision == BoundingBox_Component::CameraCollision::OUTSIDE) {
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