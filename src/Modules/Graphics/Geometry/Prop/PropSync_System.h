#pragma once
#ifndef PROPSYNC_SYSTEM_H
#define PROPSYNC_SYSTEM_H

#include "Modules/World/ECS/ecsSystem.h"
#include "Modules/World/ECS/components.h"
#include "Modules/Graphics/Geometry/Prop/PropData.h"
#include "Utilities/GL/glad/glad.h"
#include "glm/gtx/component_wise.hpp"


/** An ECS system responsible for syncronizing prop components and sending data to the GPU. */
class PropSync_System : public BaseECSSystem {
public:
	// Public (de)Constructors
	/** Destroy this system. */
	inline ~PropSync_System() = default;
	/** Construct this system.
	@param	frameData	shared pointer of common data that changes frame-to-frame. */
	inline PropSync_System(const std::shared_ptr<PropData> & frameData)
		: m_frameData(frameData) {
		addComponentType(Renderable_Component::ID, FLAG_REQUIRED);
		addComponentType(Prop_Component::ID, FLAG_REQUIRED);
		addComponentType(Skeleton_Component::ID, FLAG_OPTIONAL);
		addComponentType(Transform_Component::ID, FLAG_OPTIONAL);
		addComponentType(BoundingSphere_Component::ID, FLAG_OPTIONAL);
	}


	// Public Interface Implementations
	inline virtual void updateComponents(const float & deltaTime, const std::vector<std::vector<BaseECSComponent*>> & components) override {
		// Resize BOTH buffers to match number of entities this frame, even though not all models have skeletons
		m_frameData->modelBuffer.resize(components.size());
		m_frameData->skeletonBuffer.resize(components.size());
		m_frameData->modelBuffer.beginWriting();
		m_frameData->skeletonBuffer.beginWriting();
		int index = 0;
		for each (const auto & componentParam in components) {
			Renderable_Component * renderableComponent = (Renderable_Component*)componentParam[0];
			Prop_Component * propComponent = (Prop_Component*)componentParam[1];
			Skeleton_Component * skeletonComponent = (Skeleton_Component*)componentParam[2];
			Transform_Component * transformComponent = (Transform_Component*)componentParam[3];
			BoundingSphere_Component * bsphereComponent = (BoundingSphere_Component*)componentParam[4];

			// Synchronize the component if it is visible
			if (renderableComponent->m_visibleAtAll && propComponent->m_model->existsYet()) {
				// Sync Transform Attributes
				float radius = 1.0f;
				if (transformComponent) {
					const auto & position = transformComponent->m_transform.m_position;
					const auto & orientation = transformComponent->m_transform.m_orientation;
					const auto & scale = transformComponent->m_transform.m_scale;
					const auto matRot = glm::mat4_cast(orientation);
					m_frameData->modelBuffer[index].mMatrix = transformComponent->m_transform.m_modelMatrix;

					// Update bounding sphere
					const glm::vec3 bboxMax_World = (propComponent->m_model->m_bboxMax * scale) + position;
					const glm::vec3 bboxMin_World = (propComponent->m_model->m_bboxMin * scale) + position;
					const glm::vec3 bboxCenter = (bboxMax_World + bboxMin_World) / 2.0f;
					const glm::vec3 bboxScale = (bboxMax_World - bboxMin_World) / 2.0f;
					glm::mat4 matTrans = glm::translate(glm::mat4(1.0f), bboxCenter);
					glm::mat4 matScale = glm::scale(glm::mat4(1.0f), bboxScale);
					glm::mat4 matFinal = (matTrans * matRot * matScale);
					m_frameData->modelBuffer[index].bBoxMatrix = matFinal;
					radius = glm::compMax(propComponent->m_model->m_radius * scale);
					propComponent->m_radius = radius;
					propComponent->m_position = propComponent->m_model->m_bboxCenter + position;
				}
				if (bsphereComponent) {
					bsphereComponent->m_radius = radius;
					bsphereComponent->m_positionOffset = propComponent->m_model->m_bboxCenter;
				}

				// Sync Animation Attributes
				if (skeletonComponent) {
					auto & bones = m_frameData->skeletonBuffer[index].bones;
					for (size_t i = 0, total = std::min(skeletonComponent->m_transforms.size(), size_t(NUM_MAX_BONES)); i < total; ++i)
						bones[i] = skeletonComponent->m_transforms[i];
				}

				// Sync Prop Attributes
				m_frameData->modelBuffer[index].materialID = propComponent->m_materialID;
				m_frameData->modelBuffer[index].skinID = propComponent->m_skin;
			}
			index++;
		}
	}


private:
	// Private Attributes
	std::shared_ptr<PropData> m_frameData;
};

#endif // PROPSYNC_SYSTEM_H