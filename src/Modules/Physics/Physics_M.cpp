#include "Physics_M.h"
#include "Engine.h"

/* Component Types Used */
#include "ECS/Components/Collider_C.h"


Physics_Module::~Physics_Module()
{
	delete m_broadphase;
	delete m_collisionConfiguration;
	delete m_dispatcher;
	delete m_solver;
	delete m_world;
}

void Physics_Module::initialize(Engine * engine)
{
	Engine_Module::initialize(engine);
	m_engine->getMessageManager().statement("Loading Module: Physics...");

	m_broadphase = new btDbvtBroadphase();
	m_collisionConfiguration = new btDefaultCollisionConfiguration();
	m_dispatcher = new btCollisionDispatcher(m_collisionConfiguration);
	m_solver = new btSequentialImpulseConstraintSolver;
	m_world = new btDiscreteDynamicsWorld(m_dispatcher, m_broadphase, m_solver, m_collisionConfiguration);
	m_world->setGravity(btVector3(0, btScalar(-9.8), 0));

	// Component Constructors
	m_engine->registerECSConstructor("Collider_Component", new Collider_Constructor(m_engine));
}

void Physics_Module::physicsFrame(const float & deltaTime)
{
	m_world->stepSimulation(deltaTime);
}