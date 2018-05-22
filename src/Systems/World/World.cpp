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
	// Temporary level loading logic until a map format is chosen
	static bool loaded = false;
	if (!loaded) {

		auto hills = m_entityFactory.getEntity(m_entityFactory.createEntity("Prop"));
		hills->receiveMessage(ECSmessage(SET_MODEL_DIR, std::string("Test\\hills.obj")));
		hills->receiveMessage(ECSmessage(SET_TRANSFORM, Transform(vec3(0, -7.5, 10), quat(1, 0, 0, 0), vec3(30))));

		auto wall1 = m_entityFactory.getEntity(m_entityFactory.createEntity("Prop"));
		wall1->receiveMessage(ECSmessage(SET_MODEL_DIR, std::string("Test\\wall.obj")));
		wall1->receiveMessage(ECSmessage(SET_MODEL_SKIN, GLuint(0)));
		wall1->receiveMessage(ECSmessage(SET_TRANSFORM, Transform(vec3(-22, -10, 0), quat(1, 0, 0, 0), vec3(2))));

		auto wall2 = m_entityFactory.getEntity(m_entityFactory.createEntity("Prop"));
		wall2->receiveMessage(ECSmessage(SET_MODEL_DIR, std::string("Test\\wall.obj")));
		wall2->receiveMessage(ECSmessage(SET_MODEL_SKIN, GLuint(1)));
		wall2->receiveMessage(ECSmessage(SET_TRANSFORM, Transform(vec3(22, -10, 0), quat(1, 0, 0, 0), vec3(2))));

		auto wall3 = m_entityFactory.getEntity(m_entityFactory.createEntity("Prop"));
		wall3->receiveMessage(ECSmessage(SET_MODEL_DIR, std::string("Test\\wall.obj")));
		wall3->receiveMessage(ECSmessage(SET_MODEL_SKIN, GLuint(1)));
		wall3->receiveMessage(ECSmessage(SET_TRANSFORM, Transform(vec3(0, -10, -22), glm::rotate(quat(1, 0, 0, 0), glm::radians(90.0f), vec3(0, 1, 0)), vec3(2))));

		auto wall4 = m_entityFactory.getEntity(m_entityFactory.createEntity("Prop"));
		wall4->receiveMessage(ECSmessage(SET_MODEL_DIR, std::string("Test\\wall.obj")));
		wall4->receiveMessage(ECSmessage(SET_MODEL_SKIN, GLuint(1)));
		wall4->receiveMessage(ECSmessage(SET_TRANSFORM, Transform(vec3(0, -10, 22), glm::rotate(quat(1, 0, 0, 0), glm::radians(90.0f), vec3(0, 1, 0)), vec3(2))));

		auto wall5 = m_entityFactory.getEntity(m_entityFactory.createEntity("Prop"));
		wall5->receiveMessage(ECSmessage(SET_MODEL_DIR, std::string("Test\\wall.obj")));
		wall5->receiveMessage(ECSmessage(SET_MODEL_SKIN, GLuint(1)));
		wall5->receiveMessage(ECSmessage(SET_TRANSFORM, Transform(vec3(20, -11, 0), glm::rotate(quat(1, 0, 0, 0), glm::radians(90.0f), vec3(0, 0, 1)), vec3(2))));

		auto wall6 = m_entityFactory.getEntity(m_entityFactory.createEntity("Prop"));
		wall6->receiveMessage(ECSmessage(SET_MODEL_DIR, std::string("Test\\wall.obj")));
		wall6->receiveMessage(ECSmessage(SET_MODEL_SKIN, GLuint(1)));
		wall6->receiveMessage(ECSmessage(SET_TRANSFORM, Transform(vec3(-44, -10, -22), glm::rotate(quat(1, 0, 0, 0), glm::radians(90.0f), vec3(0, 1, 0)), vec3(2))));

		auto wall7 = m_entityFactory.getEntity(m_entityFactory.createEntity("Prop"));
		wall7->receiveMessage(ECSmessage(SET_MODEL_DIR, std::string("Test\\wall.obj")));
		wall7->receiveMessage(ECSmessage(SET_MODEL_SKIN, GLuint(1)));
		wall7->receiveMessage(ECSmessage(SET_TRANSFORM, Transform(vec3(-44, -10, 22), glm::rotate(quat(1, 0, 0, 0), glm::radians(90.0f), vec3(0, 1, 0)), vec3(2))));

		auto sun = m_entityFactory.getEntity(m_entityFactory.createEntity("Sun"));
		sun->receiveMessage(ECSmessage(SET_LIGHT_COLOR, vec3(0.75, 0.75, 0.9)));
		sun->receiveMessage(ECSmessage(SET_LIGHT_INTENSITY, 8.0f)); // OLD INTENSITY WAS 8.0
		sun->receiveMessage(ECSmessage(SET_ORIENTATION, glm::rotate(quat(0.153046, -0.690346, 0.690346, 0.153046), glm::radians(45.0f), vec3(0,0,1))));
		
		/*auto point = m_entityFactory.getEntity(m_entityFactory.createEntity("PointLight"));
		point->receiveMessage(ECSmessage(SET_LIGHT_COLOR, vec3(1.0)));
		point->receiveMessage(ECSmessage(SET_LIGHT_INTENSITY, 15.0f));
		point->receiveMessage(ECSmessage(SET_LIGHT_RADIUS, 10.0f));
		point->receiveMessage(ECSmessage(SET_POSITION, vec3(0,0,0)));*/

		/*auto spot = m_entityFactory.getEntity(m_entityFactory.createEntity("SpotLight"));
		spot->receiveMessage(ECSmessage(SET_LIGHT_COLOR, vec3(1)));
		spot->receiveMessage(ECSmessage(SET_LIGHT_INTENSITY, 15.0f));
		spot->receiveMessage(ECSmessage(SET_LIGHT_RADIUS, 10.0f));
		spot->receiveMessage(ECSmessage(SET_LIGHT_CUTOFF, 45.0f));
		spot->receiveMessage(ECSmessage(SET_POSITION, vec3(-40, 0, 0)));
		spot->receiveMessage(ECSmessage(SET_ORIENTATION, quat(1, 0, 0, 0)));*/

		auto h = m_entityFactory.getEntity(m_entityFactory.createEntity("Prop"));
		h->receiveMessage(ECSmessage(SET_MODEL_DIR, std::string("castleWall.obj")));
		h->receiveMessage(ECSmessage(SET_MODEL_SKIN, GLuint(0)));
		h->receiveMessage(ECSmessage(SET_TRANSFORM, Transform(vec3(-6, 0, 0), quat(1,0,0,0), vec3(0.1))));

		auto m1 = m_entityFactory.getEntity(m_entityFactory.createEntity("Prop"));
		m1->receiveMessage(ECSmessage(SET_MODEL_DIR, std::string("Test\\AnimationTest.fbx")));
		m1->receiveMessage(ECSmessage(SET_MODEL_SKIN, GLuint(0)));
		m1->receiveMessage(ECSmessage(SET_TRANSFORM, Transform(vec3(-5, 0, 0))));

		auto m2 = m_entityFactory.getEntity(m_entityFactory.createEntity("Prop"));
		m2->receiveMessage(ECSmessage(SET_MODEL_DIR, std::string("Test\\AnimationTest.fbx")));
		m2->receiveMessage(ECSmessage(SET_MODEL_SKIN, GLuint(0)));
		m2->receiveMessage(ECSmessage(SET_TRANSFORM, Transform(vec3(5, 0, 0), glm::rotate(quat(1, 0, 0, 0), glm::radians(90.0f), vec3(0, 1, 0)))));

		auto m3 = m_entityFactory.getEntity(m_entityFactory.createEntity("Prop"));
		m3->receiveMessage(ECSmessage(SET_MODEL_DIR, std::string("Test\\AnimationTest.fbx")));
		m3->receiveMessage(ECSmessage(SET_MODEL_SKIN, GLuint(0)));
		m3->receiveMessage(ECSmessage(SET_TRANSFORM, Transform(vec3(2, 0, -5))));

		auto m4 = m_entityFactory.getEntity(m_entityFactory.createEntity("Prop"));
		m4->receiveMessage(ECSmessage(SET_MODEL_DIR, std::string("Test\\AnimationTest.fbx")));
		m4->receiveMessage(ECSmessage(SET_MODEL_SKIN, GLuint(0)));
		m4->receiveMessage(ECSmessage(SET_TRANSFORM, Transform(vec3(0, 0, 5), glm::rotate(quat(1, 0, 0, 0), glm::radians(90.0f), vec3(0, 1, 0)))));

		auto m5 = m_entityFactory.getEntity(m_entityFactory.createEntity("Prop"));
		m5->receiveMessage(ECSmessage(SET_MODEL_DIR, std::string("Test\\AnimationTest.fbx")));
		m5->receiveMessage(ECSmessage(SET_MODEL_SKIN, GLuint(0)));
		m5->receiveMessage(ECSmessage(SET_TRANSFORM, Transform(vec3(0, -5, 0), glm::rotate(quat(1, 0, 0, 0), glm::radians(90.0f), vec3(0, 1, 1)))));

		auto m6 = m_entityFactory.getEntity(m_entityFactory.createEntity("Prop"));
		m6->receiveMessage(ECSmessage(SET_MODEL_DIR, std::string("Test\\AnimationTest.fbx")));
		m6->receiveMessage(ECSmessage(SET_MODEL_SKIN, GLuint(0)));
		m6->receiveMessage(ECSmessage(SET_TRANSFORM, Transform(vec3(-30, 0, 0))));
	
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

void System_World::calcVisibility(Camera & camera)
{
	const auto camBuffer = camera.getCameraBuffer();
	const float &radius = camBuffer.FarPlane;
	const vec3 &eyePos = camBuffer.EyePosition;
	Visibility_Token vis_token;

	for each (const auto &type in vector<const char *>{ "Anim_Model", "Light_Directional", "Light_Directional_Cheap", "Light_Spot", "Light_Point", "Reflector" }) {
		vector<Component*> visible_components;
		
		for each (auto component in getSpecificComponents<Component>(type))
			if (component->isVisible(radius, eyePos))
				visible_components.push_back(component);

		vis_token.insertType(type);
		vis_token[type] = visible_components;
	}	

	camera.setVisibilityToken(vis_token);
}
