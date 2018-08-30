#pragma once
#ifndef MODELBSPHERE_S_H
#define MODELBSPHERE_S_H 
#define GLM_ENABLE_EXPERIMENTAL

#include "ECS\Systems\ecsSystem.h"
#include "glm\gtx\component_wise.hpp"
#include <vector>

/* Component Types Used */
#include "ECS\Components\Prop_C.h"
#include "ECS\Components\BoundingSphere_C.h"


/** A system responsible for updating prop bounding spheres. */
class PropBSphere_System : public BaseECSSystem {
public: 
	// (de)Constructors
	~PropBSphere_System() = default;
	PropBSphere_System() : BaseECSSystem() {
		// Declare component types used
		addComponentType(Prop_Component::ID);
		addComponentType(BoundingSphere_Component::ID);
	}


	// Interface Implementation	
	virtual void updateComponents(const float & deltaTime, const std::vector< std::vector<BaseECSComponent*> > & components) override {
		for each (const auto & componentParam in components) {
			Prop_Component * propComponent = (Prop_Component*)componentParam[0];
			BoundingSphere_Component * bsphereComponent = (BoundingSphere_Component*)componentParam[1];
			if (propComponent->m_model->existsYet()) {
				std::shared_lock<std::shared_mutex> guard(propComponent->m_model->m_mutex);
				const glm::vec3 bboxMax_World = (propComponent->m_model->m_bboxMax * propComponent->m_transform.m_scale) + propComponent->m_transform.m_position;
				const glm::vec3 bboxMin_World = (propComponent->m_model->m_bboxMin * propComponent->m_transform.m_scale) + propComponent->m_transform.m_position;
				const glm::vec3 bboxCenter = (bboxMax_World + bboxMin_World) / 2.0f;
				const glm::vec3 bboxScale = (bboxMax_World - bboxMin_World) / 2.0f;
				glm::mat4 matTrans = glm::translate(glm::mat4(1.0f), bboxCenter);
				glm::mat4 matRot = glm::mat4_cast(propComponent->m_transform.m_orientation);
				glm::mat4 matScale = glm::scale(glm::mat4(1.0f), bboxScale);
				glm::mat4 matFinal = (matTrans * matRot * matScale);
				propComponent->m_data->data->bBoxMatrix = matFinal;
				bsphereComponent->m_radius = glm::compMax(propComponent->m_model->m_radius * propComponent->m_transform.m_scale);
				bsphereComponent->m_position = propComponent->m_model->m_bboxCenter + propComponent->m_transform.m_position;
			}
		}
	};
};

#endif // MODELBSPHERE_S_H