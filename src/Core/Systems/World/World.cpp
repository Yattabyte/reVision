#include "Systems\World\World.h"
#include "Utilities\Engine_Package.h"
#include "Rendering\Visibility_Token.h"
#include "Rendering\Camera.h"
#include "Systems\World\ECSmessages.h"
#include "Utilities\Transform.h"

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

		auto prop1 = m_entityFactory.CreateEntity("Prop");
		auto prop2 = m_entityFactory.CreateEntity("Prop");
		auto prop4 = m_entityFactory.CreateEntity("Prop");
		auto sun = m_entityFactory.CreateEntity("Sun");

		m_entityFactory.GetEntity(prop1)->ReceiveMessage(ECSmessage(SET_MODEL_DIR, std::string("Test\\skinbox2.obj")));
		m_entityFactory.GetEntity(prop2)->ReceiveMessage(ECSmessage(SET_MODEL_DIR, std::string("Test\\skinbox.obj")));
		m_entityFactory.GetEntity(prop2)->ReceiveMessage(ECSmessage(SET_MODEL_SKIN, GLuint(1)));
		m_entityFactory.GetEntity(prop4)->ReceiveMessage(ECSmessage(SET_MODEL_DIR, std::string("Sponza\\sponza.obj")));
		m_entityFactory.GetEntity(prop1)->ReceiveMessage(ECSmessage(SET_MODEL_TRANSFORM, Transform(vec3(-1, 2.5, 0))));
		m_entityFactory.GetEntity(prop2)->ReceiveMessage(ECSmessage(SET_MODEL_TRANSFORM, Transform(vec3(1, 2.5, 0))));
		m_entityFactory.GetEntity(prop4)->ReceiveMessage(ECSmessage(SET_MODEL_TRANSFORM, Transform(vec3(0, -2.5, 0))));

		auto Sun = m_entityFactory.GetEntity(sun);
		Sun->ReceiveMessage(ECSmessage(SET_LIGHT_COLOR, vec3(1, 0.75, 0.25)));
		Sun->ReceiveMessage(ECSmessage(SET_LIGHT_ORIENTATION, quat(0.153046, -0.690346, 0.690346, 0.153046)));
		Sun->ReceiveMessage(ECSmessage(SET_LIGHT_INTENSITY, 8.5f));

		m_Initialized = true;
	}
}

void System_World::Update(const float & deltaTime)
{
	/*auto Sun = m_entityFactory.GetEntity(sun);
	ori = glm::rotate(ori, glm::radians(1.0f), vec3(0, 1, 1));
	Sun->ReceiveMessage(ECSmessage(SET_LIGHT_ORIENTATION, ori));*/
	//m_entityFactory.GetEntitiesByType("Sun")[0]->ReceiveMessage(ECSmessage(SET_LIGHT_ORIENTATION, quat(0.153046, -0.690346, 0.690346, 0.153046)));
}

#include "Entities\Components\Geometry_Component.h"
#include "Entities\Components\Lighting_Component.h"
void System_World::Update_Threaded(const float & deltaTime)
{
	Camera &camera = m_enginePackage->m_Camera;
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

			vis_token.insert(pair<char*, vector<Component*>>(type, vector<Component*>()));
			vis_token[type] = visible_components;
		}
	}
	
	{
		vector<char*> types = { "Light_Directional" };
		for each (auto type in types) {
			const auto components = *((vector<Lighting_Component*>*)(&m_componentFactory.GetComponentsByType(type)));

			vector<Component*> visible_components;

			for each (auto component in components)
				if (component->IsVisible(camPVMatrix))
					visible_components.push_back((Component*)component);

			vis_token.insert(pair<char*, vector<Component*>>(type, vector<Component*>()));
			vis_token[type] = visible_components;
		}
	}
}
