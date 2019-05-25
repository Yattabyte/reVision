#pragma once
#ifndef PHYSICS_C_H
#define PHYSICS_C_H

#include "Modules/World/ECS/ecsComponent.h"
#include "Assets/Collider.h"
#include "Utilities/Transform.h"
#include "glm/glm.hpp"


/** A component representing a basic player. */
struct Collider_Component : public ECSComponent<Collider_Component> {
	float m_restitution = 1.0f;
	float m_friction = 1.0f;
	btScalar m_mass = btScalar(0);
	Shared_Collider m_collider;
	btDefaultMotionState * m_motionState = nullptr;
	btRigidBody * m_rigidBody = nullptr;
	btConvexHullShape * m_shape = nullptr;
	Transform m_transform;
};

#endif // PHYSICS_C_H