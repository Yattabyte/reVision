#include "Physics_M.h"
#include "Engine.h"


Physics_Module::~Physics_Module()
{
	delete m_broadphase;
	delete m_collisionConfiguration;
	delete m_dispatcher;
	delete m_solver;
	delete m_world;
}

Physics_Module::Physics_Module(Engine * engine) : Engine_Module(engine)
{}

void Physics_Module::initialize()
{
	m_engine->getMessageManager().statement("Loading Module: Physics...");
	m_broadphase = new btDbvtBroadphase();
	m_collisionConfiguration = new btDefaultCollisionConfiguration();
	m_dispatcher = new btCollisionDispatcher(m_collisionConfiguration);
	m_solver = new btSequentialImpulseConstraintSolver;
	m_world = new btDiscreteDynamicsWorld(m_dispatcher, m_broadphase, m_solver, m_collisionConfiguration);
	m_world->setGravity(btVector3(0, -9.8, 0));
}

void Physics_Module::physicsFrame(const float & deltaTime)
{
	m_world->stepSimulation(deltaTime);
}