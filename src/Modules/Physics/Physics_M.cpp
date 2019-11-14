#include "Modules/Physics/Physics_M.h"
#include "Modules/Physics/ECS/TransformSync_S.h"
#include "Engine.h"


void Physics_Module::initialize(Engine* engine) noexcept
{
	Engine_Module::initialize(engine);
	m_engine->getManager_Messages().statement("Loading Module: Physics...");

	m_broadphase = new btDbvtBroadphase();
	m_collisionConfiguration = new btDefaultCollisionConfiguration();
	m_dispatcher = new btCollisionDispatcher(m_collisionConfiguration);
	m_solver = new btSequentialImpulseConstraintSolver;
	m_world = new btDiscreteDynamicsWorld(m_dispatcher, m_broadphase, m_solver, m_collisionConfiguration);
	m_world->setGravity(btVector3(0, btScalar(-9.8), 0));

	// Physics Systems
	m_physicsSystems.makeSystem<TransformSync_Phys_System>(engine, m_world);
}

void Physics_Module::deinitialize() noexcept
{
	// Update indicator
	m_engine->getManager_Messages().statement("Unloading Module: Physics...");
	*m_aliveIndicator = false;

	// Delete Bullet Physics simulation
	delete m_broadphase;
	delete m_collisionConfiguration;
	delete m_dispatcher;
	delete m_solver;
	delete m_world;
}

void Physics_Module::frameTick(ecsWorld& world, const float& deltaTime) noexcept
{
	// To Do: disable physics per object if object isn't fully initialized
	m_world->stepSimulation(deltaTime);
	world.updateSystems(m_physicsSystems, deltaTime);
}