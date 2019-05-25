#include "Modules/Physics/Physics_M.h"
#include "Engine.h"

/* Component Types Used */
#include "Modules/Physics/ECS/components.h"
#include "Modules/Physics/ECS/TransformSync_S.h"


Physics_Module::~Physics_Module()
{
	delete m_broadphase;
	delete m_collisionConfiguration;
	delete m_dispatcher;
	delete m_solver;
	delete m_world;
	m_engine->getModule_World().removeComponentType("Collider_Component");
}

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
	m_physicsSystems.addSystem(new TransformSync_Phys_System(m_world));

	// Component Constructors
	m_engine->getModule_World().addComponentType("Collider_Component", [engine](const ParamList & parameters) {
		auto * component = new Collider_Component();
		component->m_collider = Shared_Collider(engine, CastAny(parameters[0], std::string("")));
		component->m_mass = btScalar(CastAny(parameters[1], 0));
		component->m_restitution = CastAny(parameters[2], 0.0f);
		component->m_friction = CastAny(parameters[3], 0.0f);
		return std::make_pair(component->ID, component);
	});
}

void Physics_Module::frameTick(const float & deltaTime)
{
	m_world->stepSimulation(deltaTime);
	m_engine->getModule_World().updateSystems(m_physicsSystems, deltaTime);
}