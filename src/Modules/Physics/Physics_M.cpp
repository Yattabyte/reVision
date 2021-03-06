#include "Modules/Physics/Physics_M.h"
#include "Modules/Physics/ECS/PhysicsSync_System.h"
#include "Engine.h"


Physics_Module::Physics_Module(Engine& engine) :
	Engine_Module(engine),
	m_dispatcher(&m_collisionConfiguration),
	m_world(&m_dispatcher, &m_broadphase, &m_solver, &m_collisionConfiguration)
{
}

void Physics_Module::initialize()
{
	Engine_Module::initialize();
	m_engine.getManager_Messages().statement("Loading Module: Physics...");
	m_world.setGravity(btVector3(0, static_cast<btScalar>(-9.8), 0));

	// Physics Systems
	m_physicsSystems.makeSystem<PhysicsSync_System>(m_engine, m_world);
}

void Physics_Module::deinitialize()
{
	// Update indicator
	m_engine.getManager_Messages().statement("Unloading Module: Physics...");
	*m_aliveIndicator = false;
}

void Physics_Module::frameTick(ecsWorld& world, const float& deltaTime)
{
	m_world.stepSimulation(deltaTime);
	updateSystems(world, deltaTime);
}

void Physics_Module::updateSystems(ecsWorld& world, const float& deltaTime)
{
	world.updateSystems(m_physicsSystems, deltaTime);
}

btDiscreteDynamicsWorld& Physics_Module::getWorld() noexcept
{
	return m_world;
}