#pragma once
#ifndef COLLIDER_S_H
#define COLLIDER_S_H 

#include "ECS\Systems\ecsSystem.h"
#include "Engine.h"
#include "glm\glm.hpp"

/* Component Types Used */
#include "ECS\Components\Physics_C.h"
#include "ECS\Components\Prop_C.h"

/** A system responsible for updating collider components. */
class Collider_System : public BaseECSSystem {
public:
	// (de)Constructors
	~Collider_System() = default;
	Collider_System(Engine * engine) : BaseECSSystem(), m_engine(engine) {
		// Declare component types used
		addComponentType(Physics_Component::ID);
		addComponentType(Prop_Component::ID);
	}


	// Interface Implementation
	virtual void updateComponents(const float & deltaTime, const std::vector< std::vector<BaseECSComponent*> > & components) override {
		for each (const auto & componentParam in components) {
			auto * physicsComponent = (Physics_Component*)componentParam[0];
			auto * propComponent = (Prop_Component*)componentParam[1];

			if (physicsComponent->m_collider->existsYet() && propComponent->m_model->existsYet()) {
				btTransform trans;
				physicsComponent->m_motionState->getWorldTransform(trans);
				btQuaternion quat = trans.getRotation();
				btVector3 pos = trans.getOrigin();
				propComponent->m_transform.m_position = glm::vec3(pos.x(), pos.y(), pos.z());
				propComponent->m_transform.m_orientation = glm::quat(quat.w(), quat.x(), quat.y(), quat.z());
				propComponent->m_transform.update();
				propComponent->m_data->data->mMatrix = propComponent->m_transform.m_modelMatrix;
				propComponent->m_data->data->bBoxMatrix = propComponent->m_transform.m_modelMatrix;
			}
		}
	};


private:
	// Private Attributes
	Engine * m_engine = nullptr;
};

#endif // COLLIDER_S_H