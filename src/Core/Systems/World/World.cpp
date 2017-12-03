#include "Systems\World\World.h"
#include "Utilities\Engine_Package.h"


#include "Systems\World\ECSmessages.h"
#include "Utilities\Transform.h"

#include "Systems\World\Components\Geometry_Manager.h"
#include "Systems\World\Components\Lighting_Manager.h"

static quat ori = glm::rotate(quat(1, 0, 0, 0), glm::radians(75.0f), vec3(1, 1, 0));
static ECShandle sun;

System_World::~System_World()
{
}

System_World::System_World(Engine_Package * package) : 
	m_enginePackage(package),
	m_ECSmessanger(&m_entityFactory, &m_componentFactory),
	m_entityFactory(&m_ECSmessanger, &m_componentFactory),
	m_componentFactory(&m_ECSmessanger)
{
	auto prop1 = m_entityFactory.CreateEntity("Prop");
	auto prop2 = m_entityFactory.CreateEntity("Prop");
	auto prop3 = m_entityFactory.CreateEntity("Prop");
	sun = m_entityFactory.CreateEntity("Sun");

	ECSmessage msg_dir(SET_MODEL_DIR, std::string("Test\\ChamferedCube.obj"));
	m_entityFactory.GetEntity(prop1)->ReceiveMessage(msg_dir);
	m_entityFactory.GetEntity(prop2)->ReceiveMessage(msg_dir);
	m_entityFactory.GetEntity(prop3)->ReceiveMessage(msg_dir);
	m_entityFactory.GetEntity(prop1)->ReceiveMessage(ECSmessage(SET_MODEL_TRANSFORM, Transform(vec3(-2.5, -2.5, 0))));
	m_entityFactory.GetEntity(prop2)->ReceiveMessage(ECSmessage(SET_MODEL_TRANSFORM, Transform(vec3(2.5, -2.5, 0))));
	m_entityFactory.GetEntity(prop3)->ReceiveMessage(ECSmessage(SET_MODEL_TRANSFORM, Transform(vec3(0, 2.5, 0))));

	auto Sun = m_entityFactory.GetEntity(sun);
	Sun->ReceiveMessage(ECSmessage(SET_LIGHT_COLOR, vec3(1, 0.75f, 0.25f)));
	Sun->ReceiveMessage(ECSmessage(SET_LIGHT_INTENSITY, 8.5f));
}

void System_World::Update(const float & deltaTime)
{
	auto Sun = m_entityFactory.GetEntity(sun);
	ori = glm::rotate(ori, glm::radians(2.0f), vec3(0, 1, 1));
	Sun->ReceiveMessage(ECSmessage(SET_LIGHT_ORIENTATION, ori));
}

void System_World::Update_Threaded(const float & deltaTime)
{
	Geometry_Manager::CalcVisibility(m_enginePackage->m_Camera, &m_componentFactory);
	Lighting_Manager::CalcVisibility(m_enginePackage->m_Camera, &m_componentFactory);
}
