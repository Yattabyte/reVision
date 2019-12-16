#include "Modules/Graphics/Geometry/Prop/PropSync_System.h"
#include "Modules/Graphics/Geometry/Prop/PropData.h"
#include "Modules/ECS/component_types.h"


PropSync_System::PropSync_System(PropData& frameData) noexcept :
	m_frameData(frameData) 
{
	addComponentType(Prop_Component::Runtime_ID, RequirementsFlag::FLAG_REQUIRED);
	addComponentType(Skeleton_Component::Runtime_ID, RequirementsFlag::FLAG_OPTIONAL);
	addComponentType(Transform_Component::Runtime_ID, RequirementsFlag::FLAG_OPTIONAL);
	addComponentType(BoundingBox_Component::Runtime_ID, RequirementsFlag::FLAG_OPTIONAL);
}

void PropSync_System::updateComponents(const float&, const std::vector<std::vector<ecsBaseComponent*>>& components) noexcept 
{
	// Resize BOTH buffers to match number of entities this frame, even though not all models have skeletons
	m_frameData.modelBuffer.resize(components.size());
	m_frameData.skeletonBuffer.resize(components.size());
	m_frameData.modelBuffer.beginWriting();
	m_frameData.skeletonBuffer.beginWriting();
	int index = 0;
	for (const auto& componentParam : components) {
		const auto* propComponent = static_cast<Prop_Component*>(componentParam[0]);
		auto* skeletonComponent = static_cast<Skeleton_Component*>(componentParam[1]);
		const auto* transformComponent = static_cast<Transform_Component*>(componentParam[2]);
		auto* bboxComponent = static_cast<BoundingBox_Component*>(componentParam[3]);

		// Synchronize the component if it is visible
		if (propComponent->m_model->ready()) {
			// Sync Transform Attributes
			if (transformComponent) {
				const auto& position = transformComponent->m_worldTransform.m_position;
				const auto& orientation = transformComponent->m_worldTransform.m_orientation;
				const auto& scale = transformComponent->m_worldTransform.m_scale;
				const auto matRot = glm::mat4_cast(orientation);
				m_frameData.modelBuffer[index].mMatrix = transformComponent->m_worldTransform.m_modelMatrix;

				// Update bounding sphere
				const glm::vec3 bboxMax_World = (propComponent->m_model->m_bboxMax * scale) + position;
				const glm::vec3 bboxMin_World = (propComponent->m_model->m_bboxMin * scale) + position;
				const glm::vec3 bboxCenter = (bboxMax_World + bboxMin_World) / 2.0f;
				const glm::vec3 bboxScale = (bboxMax_World - bboxMin_World) / 2.0f;
				const glm::mat4 matTrans = glm::translate(glm::mat4(1.0f), bboxCenter);
				const glm::mat4 matScale = glm::scale(glm::mat4(1.0f), bboxScale);
				const glm::mat4 matFinal = (matTrans * matRot * matScale);
				m_frameData.modelBuffer[index].bBoxMatrix = matFinal;
			}
			if (bboxComponent) {
				bboxComponent->m_extent = propComponent->m_model->m_bboxScale;
				bboxComponent->m_min = propComponent->m_model->m_bboxMin;
				bboxComponent->m_max = propComponent->m_model->m_bboxMax;
				bboxComponent->m_positionOffset = propComponent->m_model->m_bboxCenter;
			}

			// Sync Animation Attributes
			if (skeletonComponent) {
				skeletonComponent->m_mesh = propComponent->m_model->m_mesh;
				auto& bones = m_frameData.skeletonBuffer[index].bones;
				const auto total = std::min(skeletonComponent->m_transforms.size(), (size_t)NUM_MAX_BONES);
				for (size_t i = 0; i < total; ++i)
					bones[i] = skeletonComponent->m_transforms[i];
			}

			// Sync Prop Attributes
			m_frameData.modelBuffer[index].materialID = propComponent->m_materialID;
			m_frameData.modelBuffer[index].skinID = propComponent->m_skin;
		}
		index++;
	}
	m_frameData.modelBuffer.endWriting();
	m_frameData.skeletonBuffer.endWriting();
}