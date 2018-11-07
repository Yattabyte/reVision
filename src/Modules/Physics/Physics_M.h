#pragma once
#ifndef PHYSICS_MODULE_H
#define PHYSICS_MODULE_H

#include "Modules\Engine_Module.h"
#include "Utilities\ECS\ecsSystem.h"
#include "btBulletCollisionCommon.h"
#include "btBulletDynamicsCommon.h"


/** A module responsible for physics. */
class Physics_Module : public Engine_Module {
public:
	// (de)Constructors
	~Physics_Module();
	Physics_Module() = default;


	// Public Interface Implementation
	virtual void initialize(Engine * engine) override;


	// Public Methods
	/** Updates the physics simulation by a single frame
	@param	deltaTime	the amount of time passed since last frame */
	void physicsFrame(const float & deltaTime);
	/** Returns a pointer to the physics-world.
	@return				the physics world. */
	inline btDiscreteDynamicsWorld * getWorld() { return m_world; }


private:
	// Private Attributes
	btBroadphaseInterface * m_broadphase = nullptr;
	btDefaultCollisionConfiguration * m_collisionConfiguration = nullptr;
	btCollisionDispatcher * m_dispatcher = nullptr;
	btSequentialImpulseConstraintSolver * m_solver = nullptr;
	btDiscreteDynamicsWorld * m_world = nullptr;
	ECSSystemList m_physicsSystems;
};

#endif // PHYSICS_MODULE_H