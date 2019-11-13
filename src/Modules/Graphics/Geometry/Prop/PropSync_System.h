#pragma once
#ifndef PROPSYNC_SYSTEM_H
#define PROPSYNC_SYSTEM_H

#include "Modules/ECS/ecsSystem.h"
#include "Modules/ECS/component_types.h"
#include "Modules/Graphics/Geometry/Prop/PropData.h"
#include <glad/glad.h>
#include "glm/gtx/component_wise.hpp"


/** An ECS system responsible for synchronizing prop components and sending data to the GPU. */
class PropSync_System final : public ecsBaseSystem {
public:
	// Public (De)Constructors
	/** Destroy this system. */
	inline ~PropSync_System() = default;
	/** Construct this system.
	@param	frameData	shared pointer of common data that changes frame-to-frame. */
	inline explicit PropSync_System(const std::shared_ptr<PropData>& frameData)
		: m_frameData(frameData) {
		addComponentType(Prop_Component::Runtime_ID, RequirementsFlag::FLAG_REQUIRED);
		addComponentType(Skeleton_Component::Runtime_ID, RequirementsFlag::FLAG_OPTIONAL);
		addComponentType(Transform_Component::Runtime_ID, RequirementsFlag::FLAG_OPTIONAL);
		addComponentType(BoundingBox_Component::Runtime_ID, RequirementsFlag::FLAG_OPTIONAL);
	}


	// Public Interface Implementations
	inline virtual void updateComponents(const float& deltaTime, const std::vector<std::vector<ecsBaseComponent*>>& components) override final {
		// Resize BOTH buffers to match number of entities this frame, even though not all models have skeletons
		m_frameData->modelBuffer.resize(components.size());
		m_frameData->skeletonBuffer.resize(components.size());
		m_frameData->modelBuffer.beginWriting();
		m_frameData->skeletonBuffer.beginWriting();
		int index = 0;
		for each (const auto & componentParam in components) {
			auto* propComponent = static_cast<Prop_Component*>(componentParam[0]);
			auto* skeletonComponent = static_cast<Skeleton_Component*>(componentParam[1]);
			auto* transformComponent = static_cast<Transform_Component*>(componentParam[2]);
			auto* bboxComponent = static_cast<BoundingBox_Component*>(componentParam[3]);

			// Synchronize the component if it is visible
			if (propComponent->m_model->existsYet()) {
				// Sync Transform Attributes
				if (transformComponent) {
					const auto& position = transformComponent->m_worldTransform.m_position;
					const auto& orientation = transformComponent->m_worldTransform.m_orientation;
					const auto& scale = transformComponent->m_worldTransform.m_scale;
					const auto matRot = glm::mat4_cast(orientation);
					m_frameData->modelBuffer[index].mMatrix = transformComponent->m_worldTransform.m_modelMatrix;

					// Update bounding sphere
					const glm::vec3 bboxMax_World = (propComponent->m_model->m_bboxMax * scale) + position;
					const glm::vec3 bboxMin_World = (propComponent->m_model->m_bboxMin * scale) + position;
					const glm::vec3 bboxCenter = (bboxMax_World + bboxMin_World) / 2.0f;
					const glm::vec3 bboxScale = (bboxMax_World - bboxMin_World) / 2.0f;
					glm::mat4 matTrans = glm::translate(glm::mat4(1.0f), bboxCenter);
					glm::mat4 matScale = glm::scale(glm::mat4(1.0f), bboxScale);
					glm::mat4 matFinal = (matTrans * matRot * matScale);
					m_frameData->modelBuffer[index].bBoxMatrix = matFinal;
					float radius = glm::compMax(propComponent->m_model->m_radius * scale);
					propComponent->m_radius = radius;
					propComponent->m_position = propComponent->m_model->m_bboxCenter + position;
				}
				if (bboxComponent) {
					bboxComponent->m_extent = propComponent->m_model->m_bboxScale;
					bboxComponent->m_min = propComponent->m_model->m_bboxMin;
					bboxComponent->m_max = propComponent->m_model->m_bboxMax;
					bboxComponent->m_positionOffset = propComponent->m_model->m_bboxCenter;
				}

				// Sync Animation Attributes
				if (skeletonComponent) {
					auto& bones = m_frameData->skeletonBuffer[index].bones;
					for (size_t i = 0, total = std::min(skeletonComponent->m_transforms.size(), size_t(NUM_MAX_BONES)); i < total; ++i)
						bones[i] = skeletonComponent->m_transforms[i];
				}

				// Sync Prop Attributes
				m_frameData->modelBuffer[index].materialID = propComponent->m_materialID;
				m_frameData->modelBuffer[index].skinID = propComponent->m_skin;
			}
			index++;
		}
		m_frameData->modelBuffer.endWriting();
		m_frameData->skeletonBuffer.endWriting();
	}


private:
	// Private Attributes
	std::shared_ptr<PropData> m_frameData;
};

#endif // PROPSYNC_SYSTEM_H