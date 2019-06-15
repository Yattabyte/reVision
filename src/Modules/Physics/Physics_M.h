#pragma once
#ifndef PHYSICS_MODULE_H
#define PHYSICS_MODULE_H

#include "Modules/Engine_Module.h"
#include "Modules/World/ECS/ecsSystem.h"
#include "btBulletCollisionCommon.h"
#include "btBulletDynamicsCommon.h"


/** A module responsible for physics. */
class Physics_Module : public Engine_Module {
public:
	// Public (de)Constructors
	~Physics_Module();
	inline Physics_Module() = default;


	// Public Interface Implementation
	/** Initialize the module. */
	virtual void initialize(Engine * engine) override;
	/** Updates the physics simulation by a single frame
	@param	deltaTime	the amount of time passed since last frame */
	virtual void frameTick(const float & deltaTime) override;


	// Public Methods
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
