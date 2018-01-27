#include "Systems\World\World.h"
#include "Systems\World\Visibility_Token.h"
#include "Utilities\Engine_Package.h"
#include "Systems\World\ECSmessages.h"
#include "Utilities\Transform.h"
#include <algorithm>

System_World::~System_World()
{
}

System_World::System_World() : 
	m_ECSmessanger(&m_entityFactory, &m_componentFactory),
	m_entityFactory(&m_ECSmessanger, &m_componentFactory)
{
	
}

void System_World::Initialize(Engine_Package * enginePackage)
{
	if (!m_Initialized) {
		m_enginePackage = enginePackage;
		m_componentFactory.Initialize(m_enginePackage, &m_ECSmessanger);	

		m_Initialized = true;
	}
}

void System_World::RegisterViewer(Camera * c)
{
	unique_lock<shared_mutex> writeGuard(m_lock);
	m_viewers.push_back(c);
}

void System_World::UnRegisterViewer(Camera * c)
{
	unique_lock<shared_mutex> writeGuard(m_lock);
	m_viewers.erase(std::remove_if(begin(m_viewers), end(m_viewers), [c](const auto *camera) {
		return (camera == c);
	}), end(m_viewers));
}

#include "Entities\Components\Geometry_Component.h"
#include "Entities\Components\Lighting_Component.h"

void System_World::Update(const float & deltaTime)
{	
	static bool loaded = false;
	if (!loaded) {
		auto prop1 = m_entityFactory.CreateEntity("Prop");
		auto prop2 = m_entityFactory.CreateEntity("Prop");
		auto prop4 = m_entityFactory.CreateEntity("Prop");
		//auto sun = m_entityFactory.CreateEntity("Sun");
		auto spot = m_entityFactory.CreateEntity("SpotLight");

		m_entityFactory.GetEntity(prop1)->ReceiveMessage(ECSmessage(SET_MODEL_DIR, std::string("Test\\skinbox2.obj")));
		m_entityFactory.GetEntity(prop2)->ReceiveMessage(ECSmessage(SET_MODEL_DIR, std::string("Test\\skinbox.obj")));
		m_entityFactory.GetEntity(prop2)->ReceiveMessage(ECSmessage(SET_MODEL_SKIN, GLuint(1)));
		m_entityFactory.GetEntity(prop4)->ReceiveMessage(ECSmessage(SET_MODEL_DIR, std::string("Sponza\\sponza.obj")));
		m_entityFactory.GetEntity(prop1)->ReceiveMessage(ECSmessage(SET_MODEL_TRANSFORM, Transform(vec3(-1, 2.5, 0))));
		m_entityFactory.GetEntity(prop2)->ReceiveMessage(ECSmessage(SET_MODEL_TRANSFORM, Transform(vec3(1, 2.5, 0))));
		m_entityFactory.GetEntity(prop4)->ReceiveMessage(ECSmessage(SET_MODEL_TRANSFORM, Transform(vec3(0, -2.5, 0))));
		m_entityFactory.GetEntity(spot)->ReceiveMessage(ECSmessage(0, vec3(0.5,0.75,1)));
		m_entityFactory.GetEntity(spot)->ReceiveMessage(ECSmessage(1, 20.0f));
		m_entityFactory.GetEntity(spot)->ReceiveMessage(ECSmessage(2, 10.0f));
		m_entityFactory.GetEntity(spot)->ReceiveMessage(ECSmessage(3, 45.0f));
		m_entityFactory.GetEntity(spot)->ReceiveMessage(ECSmessage(4, vec3(1,0,0)));

		/*auto Sun = m_entityFactory.GetEntity(sun);
		Sun->ReceiveMessage(ECSmessage(SET_LIGHT_COLOR, vec3(1, 0.75, 0.50)));
		Sun->ReceiveMessage(ECSmessage(SET_LIGHT_ORIENTATION, quat(0.153046, -0.690346, 0.690346, 0.153046)));
		Sun->ReceiveMessage(ECSmessage(SET_LIGHT_INTENSITY, 5.0f));*/
		loaded = true;
	}
}

void System_World::Update_Threaded(const float & deltaTime)
{
	calcVisibility(m_enginePackage->m_Camera);
	shared_lock<shared_mutex> readGuard(m_lock);
	for each (auto &camera in m_viewers)
		calcVisibility(*camera);
}

void System_World::calcVisibility(Camera & camera)
{
	unique_lock<shared_mutex> write_guard(camera.getDataMutex());
	Visibility_Token &vis_token = camera.GetVisibilityToken();
	const auto &camBuffer = camera.getCameraBuffer();
	const mat4 camPVMatrix = camBuffer.pMatrix * camBuffer.vMatrix;

	{
		vector<char*> types = { "Anim_Model" };
		for each (auto type in types) {
			const auto components = *((vector<Geometry_Component*>*)(&m_componentFactory.GetComponentsByType(type)));

			vector<Component*> visible_components;

			for each (auto component in components)
				if (component->IsVisible(camPVMatrix))
					visible_components.push_back((Component*)component);

			vis_token.insert(type);
			vis_token[type] = visible_components;
		}
	}

	{
		vector<char*> types = { "Light_Directional", "Light_Spot" };
		for each (auto type in types) {
			const auto components = *((vector<Lighting_Component*>*)(&m_componentFactory.GetComponentsByType(type)));

			vector<Component*> visible_components;

			for each (auto component in components)
				if (component->IsVisible(camPVMatrix))
					visible_components.push_back((Component*)component);

			vis_token.insert(type);
			vis_token[type] = visible_components;
		}

	}
}
