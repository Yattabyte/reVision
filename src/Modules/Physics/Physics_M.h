#pragma once
#ifndef PHYSICS_MODULE_H
#define PHYSICS_MODULE_H

#include "Modules/Engine_Module.h"
#include "Modules/ECS/ecsSystem.h"
#include "btBulletCollisionCommon.h"
#include "btBulletDynamicsCommon.h"
#include <memory>


/** A module responsible for physics. */
class Physics_Module final : public Engine_Module {
public:
	// Public (De)Constructors
	inline ~Physics_Module() = default;
	inline Physics_Module() = default;


	// Public Interface Implementations
	virtual void initialize(Engine* engine) override final;
	virtual void deinitialize() override final;


	// Public Methods
	/** Tick this module by a specific amount of delta time.
	@param	deltaTime		the amount of time since last frame. */
	void frameTick(ecsWorld& world, const float& deltaTime);
	/** Returns a pointer to the physics-world.
	@return					the physics world. */
	inline btDiscreteDynamicsWorld* getWorld() { return m_world; }


private:
	// Private Attributes
	btBroadphaseInterface* m_broadphase = nullptr;
	btDefaultCollisionConfiguration* m_collisionConfiguration = nullptr;
	btCollisionDispatcher* m_dispatcher = nullptr;
	btSequentialImpulseConstraintSolver* m_solver = nullptr;
	btDiscreteDynamicsWorld* m_world = nullptr;
	ecsSystemList m_physicsSystems;
	std::shared_ptr<bool> m_aliveIndicator = std::make_shared<bool>(true);
};

#endif // PHYSICS_MODULE_H
