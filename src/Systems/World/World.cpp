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
		/*auto sponza = m_entityFactory.getEntity(m_entityFactory.createEntity("Prop"));
		sponza->receiveMessage(ECSmessage(SET_MODEL_DIR, std::string("Sponza\\sponza.obj")));
		sponza->receiveMessage(ECSmessage(SET_TRANSFORM, Transform(vec3(0, -2.5, 0))));*/

		auto castle = m_entityFactory.getEntity(m_entityFactory.createEntity("Prop"));
		castle->receiveMessage(ECSmessage(SET_MODEL_DIR, std::string("castleWall.obj")));
		castle->receiveMessage(ECSmessage(SET_TRANSFORM, Transform(vec3(0, -5, -20))));

		auto hills = m_entityFactory.getEntity(m_entityFactory.createEntity("Prop"));
		hills->receiveMessage(ECSmessage(SET_MODEL_DIR, std::string("Test\\hills.obj")));
		hills->receiveMessage(ECSmessage(SET_TRANSFORM, Transform(vec3(0, -7.5, 10), quat(1, 0, 0, 0), vec3(30))));

		auto sun = m_entityFactory.getEntity(m_entityFactory.createEntity("Sun"));
		sun->receiveMessage(ECSmessage(SET_LIGHT_COLOR, vec3(0.75, 0.75, 0.9)));
		sun->receiveMessage(ECSmessage(SET_LIGHT_INTENSITY, 8.0f)); // OLD INTENSITY WAS 8.0
		sun->receiveMessage(ECSmessage(SET_ORIENTATION, glm::rotate(quat(0.153046, -0.690346, 0.690346, 0.153046), glm::radians(45.0f), vec3(0,0,1))));
		

		/*auto point = m_entityFactory.getEntity(m_entityFactory.createEntity("PointLight"));
		point->receiveMessage(ECSmessage(SET_LIGHT_COLOR, vec3(0, 0.0, 1.0)));
		point->receiveMessage(ECSmessage(SET_LIGHT_INTENSITY, 15.0f));
		point->receiveMessage(ECSmessage(SET_LIGHT_RADIUS, 10.0f));
		point->receiveMessage(ECSmessage(SET_POSITION, vec3(10, 5, -20)));*/

		for (int x = 0; x < 3; ++x) {
			auto point = m_entityFactory.getEntity(m_entityFactory.createEntity("PointLight"));
			point->receiveMessage(ECSmessage(SET_LIGHT_COLOR, vec3(0, 0.0, 1.0)));
			point->receiveMessage(ECSmessage(SET_LIGHT_INTENSITY, 15.0f));
			point->receiveMessage(ECSmessage(SET_LIGHT_RADIUS, 10.0f));
			point->receiveMessage(ECSmessage(SET_POSITION, vec3(-20, -2.5, -20 + ((x - 1) * 20))));
			
			/*auto spot = m_entityFactory.getEntity(m_entityFactory.createEntity("SpotLight"));
			spot->receiveMessage(ECSmessage(SET_LIGHT_COLOR, vec3(0.5, 1, 0.0)));
			spot->receiveMessage(ECSmessage(SET_LIGHT_INTENSITY, 15.0f));
			spot->receiveMessage(ECSmessage(SET_LIGHT_RADIUS, 10.0f));
			spot->receiveMessage(ECSmessage(SET_LIGHT_CUTOFF, 45.0f));
			spot->receiveMessage(ECSmessage(SET_POSITION, vec3(-20, -2.5, -20 + ((x-1)* 20))));
			spot->receiveMessage(ECSmessage(SET_ORIENTATION, quat(1, 0, 0, 0)));*/

			auto model1 = m_entityFactory.getEntity(m_entityFactory.createEntity("Prop"));
			model1->receiveMessage(ECSmessage(SET_MODEL_DIR, std::string("Test\\AnimationTest.fbx")));
			model1->receiveMessage(ECSmessage(SET_TRANSFORM, Transform(vec3(-5, -2.5, -20 + ((x - 1) * 20)))));
			//model1->receiveMessage(ECSmessage(SET_MODEL_ANIMATION, x));
		}
		
		for (int x = 0; x < 3; ++x) {
			auto point = m_entityFactory.getEntity(m_entityFactory.createEntity("PointLight"));
			point->receiveMessage(ECSmessage(SET_LIGHT_COLOR, vec3(0, 0.0, 1.0)));
			point->receiveMessage(ECSmessage(SET_LIGHT_INTENSITY, 15.0f));
			point->receiveMessage(ECSmessage(SET_LIGHT_RADIUS, 10.0f));
			point->receiveMessage(ECSmessage(SET_POSITION, vec3(20, -2.5, -20 + ((x - 1) * 20))));

			/*auto spot = m_entityFactory.getEntity(m_entityFactory.createEntity("SpotLight"));
			spot->receiveMessage(ECSmessage(SET_LIGHT_COLOR, vec3(0.5, 1, 0.0)));
			spot->receiveMessage(ECSmessage(SET_LIGHT_INTENSITY, 15.0f));
			spot->receiveMessage(ECSmessage(SET_LIGHT_RADIUS, 10.0f));
			spot->receiveMessage(ECSmessage(SET_LIGHT_CUTOFF, 45.0f));
			spot->receiveMessage(ECSmessage(SET_POSITION, vec3(20, -2.5, -20 + ((x - 1) * 20))));
			spot->receiveMessage(ECSmessage(SET_ORIENTATION, glm::rotate(quat(1,0,0,0), glm::radians(180.0f), vec3(0,1,0))));*/

			auto model1 = m_entityFactory.getEntity(m_entityFactory.createEntity("Prop"));
			model1->receiveMessage(ECSmessage(SET_MODEL_DIR, std::string("Test\\AnimationTest.fbx")));
			model1->receiveMessage(ECSmessage(SET_TRANSFORM, Transform(vec3(10, -2.5, -20 + ((x - 1) * 20)))));
		//	model1->receiveMessage(ECSmessage(SET_MODEL_ANIMATION, x));
		}
		
		for (int x = 0; x < 10; ++x) {
			for (int y = 0; y < 10; ++y) {
				auto model1 = m_entityFactory.getEntity(m_entityFactory.createEntity("Prop"));
				model1->receiveMessage(ECSmessage(SET_MODEL_DIR, std::string("Test\\AnimationTest.fbx")));
				model1->receiveMessage(ECSmessage(SET_TRANSFORM, Transform(vec3((x - 4.5) * 2, -2.5, -20 + ((y - 4.5) * 5)))));
				//model1->receiveMessage(ECSmessage(SET_MODEL_ANIMATION, (x + y) % 3));
			}
		}

		auto wall = m_entityFactory.getEntity(m_entityFactory.createEntity("Prop"));
		wall->receiveMessage(ECSmessage(SET_MODEL_DIR, std::string("Test\\wall.obj")));
		wall->receiveMessage(ECSmessage(SET_MODEL_SKIN, GLuint(1)));
		wall->receiveMessage(ECSmessage(SET_TRANSFORM, Transform(vec3(-25, -7.5, -20))));

		auto model4 = m_entityFactory.getEntity(m_entityFactory.createEntity("Prop"));
		model4->receiveMessage(ECSmessage(SET_MODEL_DIR, std::string("Test\\ref_test.obj")));
		model4->receiveMessage(ECSmessage(SET_TRANSFORM, Transform(vec3(-25, -7.5, -40), glm::rotate(quat(1, 0, 0, 0), glm::radians(90.0f), vec3(0, 1, 0)))));

		//spot->receiveMessage(ECSmessage(SET_ORIENTATION, glm::rotate(quat(0.153046, -0.690346, 0.690346, 0.153046), glm::radians(-45.0f), vec3(0, 0, 1))));
		
		
		/*for (int x = 0; x < 16; ++x)
			for (int y = 0; y < 16; ++y) {
				auto refl = m_entityFactory.getEntity(m_entityFactory.createEntity("Reflector"));
				refl->receiveMessage(ECSmessage(SET_POSITION, vec3(x * 4, 0, y * 4)));
			}*/

		/*auto ref1 = m_entityFactory.getEntity(m_entityFactory.createEntity("Reflector"));
		ref1->receiveMessage(ECSmessage(SET_TRANSFORM, Transform(vec3(-12, -2, 0), quat(1, 0, 0, 0), vec3(4.0F))));
		ref1->receiveMessage(ECSmessage(SET_REFLECTOR_RADIUS, 2.0f));
		auto idref2 = m_entityFactory.createEntity("Reflector");
		auto ref2 = m_entityFactory.getEntity(idref2);
		ref2->receiveMessage(ECSmessage(SET_TRANSFORM, Transform(vec3(0, -2, 0), quat(1, 0, 0, 0), vec3(4.0F))));
		ref2->receiveMessage(ECSmessage(SET_REFLECTOR_RADIUS, 3.0f));
		auto ref3 = m_entityFactory.getEntity(m_entityFactory.createEntity("Reflector"));
		ref3->receiveMessage(ECSmessage(SET_TRANSFORM, Transform(vec3(12, -2, 0), quat(1, 0, 0, 0), vec3(4.0F))));
		ref3->receiveMessage(ECSmessage(SET_REFLECTOR_RADIUS, 4.0f));

		auto mir1 = m_entityFactory.getEntity(m_entityFactory.createEntity("Prop"));
		mir1->receiveMessage(ECSmessage(SET_MODEL_DIR, std::string("Test\\MirrorTest.obj")));
		mir1->receiveMessage(ECSmessage(SET_TRANSFORM, Transform(vec3(-12, -2, 0), quat(1, 0, 0, 0), vec3(6.0F,1,20))));
		auto mir2 = m_entityFactory.getEntity(m_entityFactory.createEntity("Prop"));
		mir2->receiveMessage(ECSmessage(SET_MODEL_DIR, std::string("Test\\MirrorTest.obj")));
		mir2->receiveMessage(ECSmessage(SET_TRANSFORM, Transform(vec3(0, -2, 0), quat(1, 0, 0, 0), vec3(6.0F, 1, 20))));
		auto mir3 = m_entityFactory.getEntity(m_entityFactory.createEntity("Prop"));
		mir3->receiveMessage(ECSmessage(SET_MODEL_DIR, std::string("Test\\MirrorTest.obj")));
		mir3->receiveMessage(ECSmessage(SET_TRANSFORM, Transform(vec3(12, -2, 0), quat(1, 0, 0, 0), vec3(6.0F, 1, 20))));
		
		for (int x = 0; x < 3; ++x) {
			for (int y = 0; y < 2; ++y) {
				auto point = m_entityFactory.getEntity(m_entityFactory.createEntity("PointLight"));
				point->receiveMessage(ECSmessage(SET_LIGHT_COLOR, vec3(1, 0.0, 1.0)));
				point->receiveMessage(ECSmessage(SET_LIGHT_INTENSITY, 8.0f));
				point->receiveMessage(ECSmessage(SET_LIGHT_RADIUS, 5.0f));
				point->receiveMessage(ECSmessage(SET_POSITION, vec3((x - 1) * 25, 5, (y) * 25)));
			}
		}

		for (int x = 0; x < 3; ++x) {
			for (int y = 0; y < 2; ++y) {
				auto spot = m_entityFactory.getEntity(m_entityFactory.createEntity("SpotLight"));
				spot->receiveMessage(ECSmessage(SET_LIGHT_COLOR, vec3(0.5, 0.75, 1.0)));
				spot->receiveMessage(ECSmessage(SET_LIGHT_INTENSITY, 8.0f));
				spot->receiveMessage(ECSmessage(SET_LIGHT_RADIUS, 15.0f));
				spot->receiveMessage(ECSmessage(SET_LIGHT_CUTOFF, 45.0f));
				spot->receiveMessage(ECSmessage(SET_POSITION, vec3((x - 1) * 25, 5, (y) * 25))); 
				spot->receiveMessage(ECSmessage(SET_ORIENTATION, glm::rotate(quat(0.153046, -0.690346, 0.690346, 0.153046), glm::radians(-45.0f), vec3(0, 0, 1))));
			}
		}*/

		/*auto spot = m_entityFactory.getEntity(m_entityFactory.createEntity("SpotLight"));
		spot->receiveMessage(ECSmessage(SET_LIGHT_COLOR, vec3(1, 0.75, 0.5)));
		spot->receiveMessage(ECSmessage(SET_LIGHT_INTENSITY, 15.0f));
		spot->receiveMessage(ECSmessage(SET_LIGHT_RADIUS, 15.0f));
		spot->receiveMessage(ECSmessage(SET_LIGHT_CUTOFF, 45.0f));
		spot->receiveMessage(ECSmessage(SET_POSITION, vec3(0, 5, 0)));
		spot->receiveMessage(ECSmessage(SET_ORIENTATION, glm::rotate(quat(0.153046, -0.690346, 0.690346, 0.153046), glm::radians(-45.0f), vec3(0, 0, 1))));
		*/
		
		/*auto model1 = m_entityFactory.getEntity(m_entityFactory.createEntity("Prop"));
		model1->receiveMessage(ECSmessage(SET_MODEL_DIR, std::string("Test\\AnimationTest.fbx")));
		model1->receiveMessage(ECSmessage(SET_TRANSFORM, Transform(vec3(0, 0, -10))));
		model1->receiveMessage(ECSmessage(SET_MODEL_ANIMATION, 0));
		auto model2 = m_entityFactory.getEntity(m_entityFactory.createEntity("Prop"));
		model2->receiveMessage(ECSmessage(SET_MODEL_DIR, std::string("Test\\AnimationTest.fbx")));
		model2->receiveMessage(ECSmessage(SET_TRANSFORM, Transform(vec3(-30, 0, 0))));
		model2->receiveMessage(ECSmessage(SET_MODEL_ANIMATION, 1));
		model2->receiveMessage(ECSmessage(SET_MODEL_ANIMATION, true)); 
		auto model3 = m_entityFactory.getEntity(m_entityFactory.createEntity("Prop"));
		model3->receiveMessage(ECSmessage(SET_MODEL_DIR, std::string("Test\\AnimationTest.fbx")));
		model3->receiveMessage(ECSmessage(SET_TRANSFORM, Transform(vec3(30, 0, 0))));
		model3->receiveMessage(ECSmessage(SET_MODEL_ANIMATION, 2)); 
		auto model4 = m_entityFactory.getEntity(m_entityFactory.createEntity("Prop"));
		model4->receiveMessage(ECSmessage(SET_MODEL_DIR, std::string("Test\\ref_test.obj")));
		model4->receiveMessage(ECSmessage(SET_TRANSFORM, Transform(vec3(0, 0, -30), glm::rotate(quat(1,0,0,0), glm::radians(90.0f), vec3(0,1,0)))));

		auto wall = m_entityFactory.getEntity(m_entityFactory.createEntity("Prop"));
		wall->receiveMessage(ECSmessage(SET_MODEL_DIR, std::string("Test\\wall.obj")));
		wall->receiveMessage(ECSmessage(SET_MODEL_SKIN, GLuint(1)));
		wall->receiveMessage(ECSmessage(SET_TRANSFORM, Transform(vec3(100, -7.5, 20))));*/

		

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
			model->receiveMessage(ECSmessage(SET_TRANSFORM, Transform(vec3((x - 1)*2.5f, -2.5, 2.5), quat(1, 0, 0, 0), vec3(0.5f))));
		}
		*/
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

	for each (const auto &type in vector<const char *>{ "Anim_Model", "Light_Directional", "Light_Spot", "Light_Point", "Reflector" }) {
		vector<Component*> visible_components;
		
		for each (auto component in getSpecificComponents<Component>(type))
			if (component->isVisible(radius, eyePos))
				visible_components.push_back(component);

		vis_token.insertType(type);
		vis_token[type] = visible_components;
	}	

	camera.setVisibilityToken(vis_token);
}
