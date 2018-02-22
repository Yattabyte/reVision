#include "Systems\World\World.h"
#include "Systems\World\Visibility_Token.h"
#include "Utilities\EnginePackage.h"
#include "Systems\World\ECS\ECSdefines.h"
#include "Utilities\Transform.h"
#include <algorithm>


System_World::~System_World()
{
}

System_World::System_World() : 
	m_ECSmessenger(&m_entityFactory, &m_componentFactory),
	m_entityFactory(&m_ECSmessenger, &m_componentFactory),
	m_animator(Animator(this))
{

}

void System_World::initialize(EnginePackage * enginePackage)
{
	if (!m_Initialized) {
		m_enginePackage = enginePackage;
		m_componentFactory.initialize(m_enginePackage, &m_ECSmessenger);	

		m_Initialized = true;
	}
}

void System_World::update(const float & deltaTime)
{	
	static bool loaded = false;
	if (!loaded) {
		auto sponza = m_entityFactory.getEntity(m_entityFactory.createEntity("Prop"));
		sponza->receiveMessage(ECSmessage(SET_MODEL_DIR, std::string("Sponza\\sponza.obj")));
		sponza->receiveMessage(ECSmessage(SET_MODEL_TRANSFORM, Transform(vec3(0, -2.5, 0))));

		auto hills = m_entityFactory.getEntity(m_entityFactory.createEntity("Prop"));
		hills->receiveMessage(ECSmessage(SET_MODEL_DIR, std::string("Test\\hills.obj")));
		hills->receiveMessage(ECSmessage(SET_MODEL_TRANSFORM, Transform(vec3(0, -7.5, 10), quat(1, 0, 0, 0), vec3(30))));

		/*for (int x = 0; x < 3; ++x) {
			for (int y = 0; y < 2; ++y) {
				auto point = m_entityFactory.getEntity(m_entityFactory.createEntity("PointLight"));
				point->receiveMessage(ECSmessage(SET_LIGHT_COLOR, vec3(1, 0.75, 0.5)));
				point->receiveMessage(ECSmessage(SET_LIGHT_INTENSITY, 5.0f));
				point->receiveMessage(ECSmessage(SET_LIGHT_RADIUS, 5.0f));
				point->receiveMessage(ECSmessage(SET_POSITION, vec3((x - 1) * 25, 5, (y) * 25)));;
			}
		}*/
		auto sun = m_entityFactory.getEntity(m_entityFactory.createEntity("Sun"));
		sun->receiveMessage(ECSmessage(SET_LIGHT_COLOR, vec3(1, 0.80, 0.6)));
		sun->receiveMessage(ECSmessage(SET_LIGHT_INTENSITY, 5.5f));
		sun->receiveMessage(ECSmessage(SET_ORIENTATION, glm::rotate(quat(0.153046, -0.690346, 0.690346, 0.153046), glm::radians(-45.0f), vec3(0,0,1))));
			
		/*auto model1 = m_entityFactory.getEntity(m_entityFactory.createEntity("Prop"));
		model1->receiveMessage(ECSmessage(SET_MODEL_DIR, std::string("Test\\AnimationTest.fbx")));
		model1->receiveMessage(ECSmessage(SET_MODEL_TRANSFORM, Transform(vec3(0, 0, -10))));
		model1->receiveMessage(ECSmessage(SET_MODEL_ANIMATION, 0));
		auto model2 = m_entityFactory.getEntity(m_entityFactory.createEntity("Prop"));
		model2->receiveMessage(ECSmessage(SET_MODEL_DIR, std::string("Test\\AnimationTest.fbx")));
		model2->receiveMessage(ECSmessage(SET_MODEL_TRANSFORM, Transform(vec3(-30, 0, 0))));
		model2->receiveMessage(ECSmessage(SET_MODEL_ANIMATION, 1));
		model2->receiveMessage(ECSmessage(SET_MODEL_ANIMATION, true)); 
		auto model3 = m_entityFactory.getEntity(m_entityFactory.createEntity("Prop"));
		model3->receiveMessage(ECSmessage(SET_MODEL_DIR, std::string("Test\\AnimationTest.fbx")));
		model3->receiveMessage(ECSmessage(SET_MODEL_TRANSFORM, Transform(vec3(30, 0, 0))));
		model3->receiveMessage(ECSmessage(SET_MODEL_ANIMATION, 2));*/

		auto wall = m_entityFactory.getEntity(m_entityFactory.createEntity("Prop"));
		wall->receiveMessage(ECSmessage(SET_MODEL_DIR, std::string("Test\\wall.obj")));
		wall->receiveMessage(ECSmessage(SET_MODEL_SKIN, GLuint(1)));
		wall->receiveMessage(ECSmessage(SET_MODEL_TRANSFORM, Transform(vec3(100, -7.5, 20))));
		wall->receiveMessage(ECSmessage(SET_MODEL_ANIMATION, 2));

		

		/*for (int x = 0; x < 3; ++x) {
			auto spot = m_entityFactory.GetEntity(m_entityFactory.CreateEntity("SpotLight"));
			spot->receiveMessage(ECSmessage(0, vec3(0.5, 0.75, 1)));
			spot->receiveMessage(ECSmessage(1, 20.0f));
			spot->receiveMessage(ECSmessage(2, 10.0f));
			spot->receiveMessage(ECSmessage(3, 45.0f));
			spot->receiveMessage(ECSmessage(4, vec3(x-1, 0, 5)));
			spot->receiveMessage(ECSmessage(5, glm::rotate(quat(1,0,0,0), glm::radians((x-1)*90.0f), vec3(0, 1, 0))));
			auto model = m_entityFactory.GetEntity(m_entityFactory.CreateEntity("Prop"));
			model->receiveMessage(ECSmessage(SET_MODEL_DIR, std::string("Test\\AnimationTest.fbx")));
			model->receiveMessage(ECSmessage(SET_MODEL_TRANSFORM, Transform(vec3((x - 1)*2.5f, -2.5, 2.5), quat(1, 0, 0, 0), vec3(0.5f))));
		}

		/*auto sun = m_entityFactory.GetEntity(m_entityFactory.CreateEntity("Sun"));				
		sun->receiveMessage(ECSmessage(SET_LIGHT_COLOR, vec3(1, 0.75, 0.50)));
		sun->receiveMessage(ECSmessage(SET_LIGHT_ORIENTATION, quat(0.153046, -0.690346, 0.690346, 0.153046)));
		sun->receiveMessage(ECSmessage(SET_LIGHT_INTENSITY, 5.0f));*/
		loaded = true;
	}

	m_animator.animate(deltaTime);
}

void System_World::updateThreaded(const float & deltaTime)
{
	calcVisibility(m_enginePackage->m_Camera);
	shared_lock<shared_mutex> readGuard(m_lock);
	for each (auto &camera in m_viewers)
		calcVisibility(*camera);
}

void System_World::registerViewer(Camera * c)
{
	unique_lock<shared_mutex> writeGuard(m_lock);
	m_viewers.push_back(c);
}

void System_World::unregisterViewer(Camera * c)
{
	unique_lock<shared_mutex> writeGuard(m_lock);
	m_viewers.erase(std::remove_if(begin(m_viewers), end(m_viewers), [c](const auto * camera) {
		return (camera == c);
	}), end(m_viewers));
}

#include "Systems\World\ECS\Components\Geometry_Component.h"
#include "Systems\World\ECS\Components\Lighting_Component.h"
void System_World::calcVisibility(Camera & camera)
{
	unique_lock<shared_mutex> write_guard(camera.getDataMutex());
	Visibility_Token &vis_token = camera.GetVisibilityToken();
	const auto &camBuffer = camera.getCameraBuffer();
	const mat4 &camPMatrix = camBuffer.pMatrix;
	const mat4 &camVMatrix = camBuffer.vMatrix;

	{
		vector<const char *> types = { "Anim_Model" };
		for each (auto type in types) {
			const auto &components = getSpecificComponents<Geometry_Component>(type);

			vector<Component*> visible_components;

			for each (auto component in components)
				if (component->isVisible(camPMatrix, camVMatrix))
					visible_components.push_back((Component*)component);

			vis_token.insertType(type);
			vis_token[type] = visible_components;
		}
	}

	{
		vector<const char *> types = { "Light_Directional", "Light_Spot", "Light_Point" };
		for each (auto type in types) {
			const auto &components = getSpecificComponents<Lighting_Component>(type);

			vector<Component*> visible_components;

			for each (auto component in components)
				if (component->isVisible(camPMatrix, camVMatrix))
					visible_components.push_back((Component*)component);

			vis_token.insertType(type);
			vis_token[type] = visible_components;
		}

	}
}
