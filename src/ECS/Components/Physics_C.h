#pragma once
#ifndef PHYSICS_C_H
#define PHYSICS_C_H

#include "ECS\Components\ecsComponent.h"
#include "Assets\Asset_Collider.h"
#include "Utilities\Transform.h"
#include "glm\glm.hpp"


/** A component representing a basic player. */
struct Physics_Component : public ECSComponent<Physics_Component> {
	Shared_Asset_Collider m_collider;
	btDefaultMotionState * m_motionState = nullptr;
};
/** A constructor to aid in creation. */
struct Physics_Constructor : ECSComponentConstructor<Physics_Component> {
	Physics_Constructor(Engine * engine, btDiscreteDynamicsWorld * world)
		: m_engine(engine), m_world(world)
	{};
	// Interface Implementation
	virtual Component_and_ID construct(const std::vector<std::any> & parameters) override {
		const auto directory = castAny(parameters[0], std::string(""));
		const auto mass = btScalar(castAny(parameters[1], 0));
		const auto restitution = castAny(parameters[2], 0.0f);
		const auto friction = castAny(parameters[3], 0.0f);
		const auto position = castAny(parameters[4], glm::vec3(0.0f));
		const auto orientation = castAny(parameters[5], glm::quat(1, 0, 0, 0));
		const auto scale = castAny(parameters[6], glm::vec3(1.0f));
		auto * component = new Physics_Component();
		auto motionState = new btDefaultMotionState(btTransform(btQuaternion(orientation.x, orientation.y, orientation.z, orientation.w), btVector3(position.x, position.y, position.z)));
		component->m_motionState = motionState;
		auto collider = Asset_Collider::Create(m_engine, directory);
		component->m_collider = collider;
		component->m_collider->addCallback(m_aliveIndicator, [&, mass, restitution, collider, motionState, scale]() {
			collider->m_shape->setLocalScaling(btVector3(scale.x, scale.y, scale.z));
			btVector3 Inertia(0, 0, 0);
			collider->m_shape->calculateLocalInertia(mass, Inertia);
			auto bodyCI = btRigidBody::btRigidBodyConstructionInfo(mass, motionState, collider->m_shape, Inertia);
			bodyCI.m_restitution = restitution;
			bodyCI.m_friction = friction;
			m_world->addRigidBody(new btRigidBody(bodyCI));
		});
		return { component, component->ID };
		
	}
	Engine * m_engine = nullptr;
	btDiscreteDynamicsWorld * m_world = nullptr;
	std::shared_ptr<bool> m_aliveIndicator = std::make_shared<bool>(true);
};

#endif // PHYSICS_C_H