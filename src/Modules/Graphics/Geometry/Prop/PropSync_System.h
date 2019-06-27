#pragma once
#ifndef PROPSYNC_SYSTEM_H
#define PROPSYNC_SYSTEM_H

#include "Modules/World/ECS/ecsSystem.h"
#include "Modules/World/ECS/components.h"
#include "Modules/Graphics/Logical/components.h"
#include "Modules/Graphics/Geometry/components.h"
#include "Utilities/GL/GL_ArrayBuffer.h"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtx/component_wise.hpp"
#include <memory>

#define NUM_MAX_BONES 100


/***/
struct Prop_Buffers {
	/** OpenGL buffer for models. */
	struct Model_Buffer {
		GLuint materialID;
		GLuint isStatic; glm::vec2 padding1;
		glm::mat4 mMatrix;
		glm::mat4 bBoxMatrix;
	};
	/** OpenGL buffer for prop skeletons. */
	struct Skeleton_Buffer {
		glm::mat4 bones[NUM_MAX_BONES];
	};
	GL_ArrayBuffer<Model_Buffer> modelBuffer;
	GL_ArrayBuffer<Skeleton_Buffer> skeletonBuffer;
};

/***/
class PropSync_System : public BaseECSSystem {
public:
	// Public (de)Constructors
	/***/
	inline ~PropSync_System() = default;
	/***/
	inline PropSync_System(const std::shared_ptr<Prop_Buffers> & buffers)
		: m_buffers(buffers) {
		addComponentType(Renderable_Component::ID, FLAG_REQUIRED);
		addComponentType(Prop_Component::ID, FLAG_REQUIRED);
		addComponentType(Skeleton_Component::ID, FLAG_OPTIONAL);
		addComponentType(Transform_Component::ID, FLAG_OPTIONAL);
		addComponentType(BoundingSphere_Component::ID, FLAG_OPTIONAL);
	}


	// Public Interface Implementations
	inline virtual void updateComponents(const float & deltaTime, const std::vector<std::vector<BaseECSComponent*>> & components) override {
		for each (const auto & componentParam in components) {
			Renderable_Component * renderableComponent = (Renderable_Component*)componentParam[0];
			Prop_Component * propComponent = (Prop_Component*)componentParam[1];
			Skeleton_Component * skeletonComponent = (Skeleton_Component*)componentParam[2];
			Transform_Component * transformComponent = (Transform_Component*)componentParam[3];
			BoundingSphere_Component * bsphereComponent = (BoundingSphere_Component*)componentParam[4];
			const auto & index = propComponent->m_modelBufferIndex;

			// Synchronize the component if it is visible
			if (renderableComponent->m_visibleAtAll && propComponent->m_model->existsYet()) {
				// Sync Transform Attributes
				float radius = 1.0f;
				if (transformComponent) {
					const auto & position = transformComponent->m_transform.m_position;
					const auto & orientation = transformComponent->m_transform.m_orientation;
					const auto & scale = transformComponent->m_transform.m_scale;
					const auto matRot = glm::mat4_cast(orientation);
					m_buffers->modelBuffer[index].mMatrix = transformComponent->m_transform.m_modelMatrix;

					// Update bounding sphere
					const glm::vec3 bboxMax_World = (propComponent->m_model->m_bboxMax * scale) + position;
					const glm::vec3 bboxMin_World = (propComponent->m_model->m_bboxMin * scale) + position;
					const glm::vec3 bboxCenter = (bboxMax_World + bboxMin_World) / 2.0f;
					const glm::vec3 bboxScale = (bboxMax_World - bboxMin_World) / 2.0f;
					glm::mat4 matTrans = glm::translate(glm::mat4(1.0f), bboxCenter);
					glm::mat4 matScale = glm::scale(glm::mat4(1.0f), bboxScale);
					glm::mat4 matFinal = (matTrans * matRot * matScale);
					m_buffers->modelBuffer[index].bBoxMatrix = matFinal;
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
					propComponent->m_static = false;
					auto & bones = m_buffers->skeletonBuffer[skeletonComponent->m_skeleBufferIndex].bones;
					for (size_t i = 0, total = std::min(skeletonComponent->m_transforms.size(), size_t(NUM_MAX_BONES)); i < total; ++i)
						bones[i] = skeletonComponent->m_transforms[i];
				}

				// Sync Prop Attributes
				m_buffers->modelBuffer[index].materialID = propComponent->m_skin;
				m_buffers->modelBuffer[index].isStatic = propComponent->m_static;
			}
		}
	}

private:
	// Private Attributes
	std::shared_ptr<Prop_Buffers> m_buffers;
};

#endif // PROPSYNC_SYSTEM_H