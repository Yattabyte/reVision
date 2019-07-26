#include "Modules/Physics/Physics_M.h"
#include "Modules/Physics/ECS/TransformSync_S.h"
#include "Engine.h"


void Physics_Module::initialize(Engine * engine)
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
	m_physicsSystems.addSystem(new TransformSync_Phys_System(engine, m_world));

	// World-Changed Callback
	m_engine->getModule_World().addLevelListener(m_aliveIndicator, [&](const World_Module::WorldState & state) {
		if (state == World_Module::unloaded)
			m_enabled = false;
		else if (state == World_Module::finishLoading || state == World_Module::updated)
			m_enabled = true;
	});
}

void Physics_Module::deinitialize()
{
	// Update indicator
	m_engine->getManager_Messages().statement("Unloading Module: Physics...");
	m_aliveIndicator = false;

	// Delete Bullet Physics simulation
	delete m_broadphase;
	delete m_collisionConfiguration;
	delete m_dispatcher;
	delete m_solver;
	delete m_world;	
}

void Physics_Module::frameTick(const float & deltaTime)
{
	if (m_enabled) {
		// Only update simulation if engine is READY
		m_world->stepSimulation(deltaTime);
		m_engine->getModule_World().updateSystems(m_physicsSystems, deltaTime);
	}
}