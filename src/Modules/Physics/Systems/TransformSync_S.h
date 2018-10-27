#pragma once
#ifndef TRANSFORMSYNC_PHYS_S_H
#define TRANSFORMSYNC_PHYS_S_H
#define GLM_ENABLE_EXPERIMENTAL

#include "Utilities\ECS\ecsSystem.h"
#include "Engine.h"
#include "glm\glm.hpp"
#include "glm\gtx\component_wise.hpp"

/* Component Types Used */
#include "Utilities\ECS\Transform_C.h"
#include "Modules\Physics\Components\Collider_C.h"


/** A system responsible for updating physics components that share a common transformation. */
class TransformSync_Phys_System : public BaseECSSystem {
public:
	// (de)Constructors
	~TransformSync_Phys_System() = default;
	TransformSync_Phys_System(btDiscreteDynamicsWorld * world)
		: BaseECSSystem(), m_world(world) 
	{
		// Declare component types used
		addComponentType(Transform_Component::ID);
		addComponentType(Collider_Component::ID, FLAG_OPTIONAL);
	}


	// Interface Implementation
	virtual void updateComponents(const float & deltaTime, const std::vector< std::vector<BaseECSComponent*> > & components) override {
		for each (const auto & componentParam in components) {
			auto * transformComponent = (Transform_Component*)componentParam[0];
			auto * colliderComponent = (Collider_Component*)componentParam[1];

			const auto & position = transformComponent->m_transform.m_position;
			const auto & orientation = transformComponent->m_transform.m_orientation;
			const auto & scale = transformComponent->m_transform.m_scale;
			const auto & modelMatrix = transformComponent->m_transform.m_modelMatrix;

			if (colliderComponent) {
				if (colliderComponent->m_collider->existsYet()) {
					// If the collider's transformation is out of date
					if (colliderComponent->m_transform != transformComponent->m_transform) {

						// Remove from the physics simulation
						if (colliderComponent->m_rigidBody) {
							m_world->removeRigidBody(colliderComponent->m_rigidBody);
							delete colliderComponent->m_rigidBody;
						}

						// Build the collider from the transform info
						if (!colliderComponent->m_motionState)
							colliderComponent->m_motionState = new btDefaultMotionState();
						colliderComponent->m_motionState->setWorldTransform(btTransform(btTransform(btQuaternion(orientation.x, orientation.y, orientation.z, orientation.w), btVector3(position.x, position.y, position.z))));

						// Resize the collider shape to fit
						if (colliderComponent->m_transform.m_scale != transformComponent->m_transform.m_scale || !colliderComponent->m_shape) {	
							if (!colliderComponent->m_shape)
								delete colliderComponent->m_shape;
							colliderComponent->m_shape = new btConvexHullShape(*(btConvexHullShape*)colliderComponent->m_collider->m_shape);
							colliderComponent->m_shape->setLocalScaling(btVector3(scale.x, scale.y, scale.z));
						}

						// Add back to simulation
						btVector3 Inertia(0, 0, 0);
						colliderComponent->m_shape->calculateLocalInertia(colliderComponent->m_mass, Inertia);
						auto bodyCI = btRigidBody::btRigidBodyConstructionInfo(colliderComponent->m_mass, colliderComponent->m_motionState, colliderComponent->m_shape, Inertia);
						bodyCI.m_restitution = colliderComponent->m_restitution;
						bodyCI.m_friction = colliderComponent->m_friction;
						colliderComponent->m_rigidBody = new btRigidBody(bodyCI);
						m_world->addRigidBody(colliderComponent->m_rigidBody);

						// Update the transform
						colliderComponent->m_transform = transformComponent->m_transform;
					}
					
					// Otherwise update the transform with the collider info
					else {
						btTransform trans;
						colliderComponent->m_motionState->getWorldTransform(trans);
						const btQuaternion quat = trans.getRotation();
						const btVector3 pos = trans.getOrigin();
						transformComponent->m_transform.m_position = glm::vec3(pos.x(), pos.y(), pos.z());
						transformComponent->m_transform.m_orientation = glm::quat(quat.w(), quat.x(), quat.y(), quat.z());
						transformComponent->m_transform.update();
						colliderComponent->m_transform = transformComponent->m_transform;
					}
				}
			}
		}
	};


private:
	// Private Attributes
	btDiscreteDynamicsWorld * m_world = nullptr;
};

#endif // TRANSFORMSYNC_PHYS_S_H