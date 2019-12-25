#pragma once
#ifndef PHYSICS_MODULE_H
#define PHYSICS_MODULE_H

#include "Modules/Engine_Module.h"
#include "Modules/ECS/ecsSystem.h"
#include "btBulletCollisionCommon.h"
#include "btBulletDynamicsCommon.h"


/** A module responsible for physics. */
class Physics_Module final : public Engine_Module {
public:
	// Public (De)Constructors
	/** Construct a physics module.
	@param	engine		reference to the engine to use. */
	explicit Physics_Module(Engine& engine);


	// Public Interface Implementations
	void initialize() final;
	void deinitialize() final;


	// Public Methods
	/** Tick this module by a specific amount of delta time.
	@param	deltaTime	the amount of time since last frame. */
	void frameTick(ecsWorld& world, const float& deltaTime);
	/** Update generic physics based ECS systems using a specific ECS world. */
	void updateSystems(ecsWorld& world, const float& deltaTime);
	/** Retrieves a pointer to the physics-world.
	@return				reference to the physics world. */
	btDiscreteDynamicsWorld& getWorld() noexcept;


private:
	// Private Attributes
	btDbvtBroadphase m_broadphase;
	btDefaultCollisionConfiguration m_collisionConfiguration;
	btCollisionDispatcher m_dispatcher;
	btSequentialImpulseConstraintSolver m_solver;
	btDiscreteDynamicsWorld m_world;
	ecsSystemList m_physicsSystems;
	std::shared_ptr<bool> m_aliveIndicator = std::make_shared<bool>(true);
};

#endif // PHYSICS_MODULE_H