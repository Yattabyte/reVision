#include "Modules/Physics/ECS/PhysicsSync_System.h"
#include "Modules/ECS/component_types.h"
#include "glm/glm.hpp"


PhysicsSync_System::PhysicsSync_System(Engine& engine, btDiscreteDynamicsWorld& world) :
	
	m_engine(engine),
	m_world(world)
{
	// Declare component types used
	addComponentType(Transform_Component::Runtime_ID);
	addComponentType(Collider_Component::Runtime_ID, RequirementsFlag::FLAG_OPTIONAL);
}

void PhysicsSync_System::updateComponents(const float& /*deltaTime*/, const std::vector<std::vector<ecsBaseComponent*>>& components) 
{
	for (const auto& componentParam : components) {
		auto* transformComponent = static_cast<Transform_Component*>(componentParam[0]);
		auto* colliderComponent = static_cast<Collider_Component*>(componentParam[1]);

		const auto& position = transformComponent->m_worldTransform.m_position;
		const auto& orientation = transformComponent->m_worldTransform.m_orientation;
		const auto& scale = transformComponent->m_worldTransform.m_scale;

		if (colliderComponent != nullptr) {
			if (!colliderComponent->m_collider)
				colliderComponent->m_collider = Shared_Collider(m_engine, colliderComponent->m_modelName);
			else if (colliderComponent->m_collider->ready()) {
				// If the collider's transformation is out of date
				if (colliderComponent->m_worldTransform != transformComponent->m_localTransform) {
					// Remove from the physics simulation
					if (colliderComponent->m_rigidBody) {
						m_world.removeRigidBody(colliderComponent->m_rigidBody.get());
						colliderComponent->m_rigidBody.reset();
					}

					// Build the collider from the transform info
					colliderComponent->m_motionState->setWorldTransform(btTransform(btTransform(btQuaternion(orientation.x, orientation.y, orientation.z, orientation.w), btVector3(position.x, position.y, position.z))));

					// Resize the collider shape to fit
					if (colliderComponent->m_worldTransform.m_scale != transformComponent->m_worldTransform.m_scale || !colliderComponent->m_shape) {
						colliderComponent->m_shape = std::make_shared<btConvexHullShape>(*dynamic_cast<btConvexHullShape*>(colliderComponent->m_collider->m_shape.get()));
						colliderComponent->m_shape->setLocalScaling(btVector3(scale.x, scale.y, scale.z));
					}

					// Add back to simulation
					btVector3 Inertia(0, 0, 0);
					colliderComponent->m_shape->calculateLocalInertia(colliderComponent->m_mass, Inertia);
					auto bodyCI = btRigidBody::btRigidBodyConstructionInfo(colliderComponent->m_mass, colliderComponent->m_motionState.get(), colliderComponent->m_shape.get(), Inertia);
					bodyCI.m_restitution = colliderComponent->m_restitution;
					bodyCI.m_friction = colliderComponent->m_friction;
					colliderComponent->m_rigidBody = std::make_shared<btRigidBody>(bodyCI);
					m_world.addRigidBody(colliderComponent->m_rigidBody.get());

					// Update the transform
					colliderComponent->m_worldTransform = transformComponent->m_worldTransform;
				}
				// Otherwise update the transform with the collider info
				else {
					btTransform trans;
					colliderComponent->m_motionState->getWorldTransform(trans);
					const auto quat = trans.getRotation();
					const auto pos = trans.getOrigin();
					transformComponent->m_localTransform.m_position = glm::vec3(pos.x(), pos.y(), pos.z());
					transformComponent->m_localTransform.m_orientation = glm::quat(quat.w(), quat.x(), quat.y(), quat.z());
					transformComponent->m_localTransform.update();
					colliderComponent->m_worldTransform = transformComponent->m_localTransform;
				}
			}
		}
	}
}