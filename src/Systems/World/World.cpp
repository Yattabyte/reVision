#include "Systems\World\World.h"
#include "Systems\World\Visibility_Token.h"
#include "Engine.h"
#include "Systems\World\ECS\ECS_DEFINES.h"
#include "Utilities\Transform.h"
#include <algorithm>


System_World::~System_World()
{
}

System_World::System_World() : 
	m_entityFactory(&m_componentFactory),
	m_animator(Animator(this))
{
	m_loaded = false;
	m_worldChanged = false;
}

void System_World::initialize(Engine * engine)
{
	if (!m_Initialized) {
		m_engine = engine;
		m_componentFactory.initialize(m_engine);	

		m_Initialized = true;
	}
}

void System_World::update(const float & deltaTime)
{
	checkWorld();
	loadWorld();
	m_animator.animate(deltaTime);
}

void System_World::updateThreaded(const float & deltaTime)
{
	shared_lock<shared_mutex> stateGuard(m_stateLock);
	//if (!m_worldChanged && m_loaded) {
		calcVisibility(*m_engine->getCamera());
		shared_lock<shared_mutex> viewerGuard(m_viewerLock);
		for each (auto &camera in m_viewers)
			calcVisibility(*camera);
	//}
}

void System_World::registerViewer(Camera * c)
{
	unique_lock<shared_mutex> writeGuard(m_viewerLock);
	m_viewers.push_back(c);
}

void System_World::unregisterViewer(Camera * c)
{
	unique_lock<shared_mutex> writeGuard(m_viewerLock);
	m_viewers.erase(std::remove_if(begin(m_viewers), end(m_viewers), [c](const auto * camera) {
		return (camera == c);
	}), end(m_viewers));
}

void System_World::notifyWhenLoaded(bool * notifyee)
{
	m_loadNotifiers.push_back(notifyee);
}

void System_World::calcVisibility(Camera & camera)
{
	const auto camBuffer = camera.getCameraBuffer();
	const float &radius = camBuffer.FarPlane;
	const vec3 &eyePos = camBuffer.EyePosition;
	Visibility_Token vis_token;

	for each (const auto &type in vector<const char *>{ "Static_Model", "Anim_Model", "Light_Directional", "Light_Directional_Cheap", "Light_Spot", "Light_Spot_Cheap", "Light_Point", "Light_Point_Cheap", "Reflector" }) {
		vector<Component*> visible_components;
		
		for each (auto component in getSpecificComponents<Component>(type))
			if (component->isVisible(radius, eyePos))
				visible_components.push_back(component);

		vis_token.insertType(type);
		vis_token[type] = visible_components;
	}	

	camera.setVisibilityToken(vis_token);
}

void System_World::loadWorld()
{
	// Temporary level loading logic until a map format is chosen
	static bool temp_loaded = false;
	if (!temp_loaded) {
		Entity * hills = m_entityFactory.createEntity("Prop");
		hills->sendCommand("Load_Model", string("Test\\hills.obj"));
		hills->sendCommand("Change_Transform", Transform(vec3(0, -7.5, 10), quat(1, 0, 0, 0), vec3(30)));

		Entity * wall1 = m_entityFactory.createEntity("Prop");
		wall1->sendCommand("Load_Model", string("Test\\wall.obj"));
		wall1->sendCommand("Change_Skin", 0);
		wall1->sendCommand("Change_Transform", Transform(vec3(-22, -10, 0), quat(1, 0, 0, 0), vec3(2)));

		Entity * wall2 = m_entityFactory.createEntity("Prop");
		wall2->sendCommand("Load_Model", string("Test\\wall.obj"));
		wall2->sendCommand("Change_Skin", 2);
		wall2->sendCommand("Change_Transform", Transform(vec3(22, -10, 0), quat(1, 0, 0, 0), vec3(2)));

		Entity * wall3 = m_entityFactory.createEntity("Prop");
		wall3->sendCommand("Load_Model", string("Test\\wall.obj"));
		wall3->sendCommand("Change_Skin", 1);
		wall3->sendCommand("Change_Transform", Transform(vec3(0, -10, -22), glm::rotate(quat(1, 0, 0, 0), glm::radians(90.0f), vec3(0, 1, 0)), vec3(2)));

		/*	Entity * wall4 = m_entityFactory.createEntity("Prop");
		wall4->sendCommand("Load_Model", string("Test\\wall.obj"));
		wall4->sendCommand("Change_Skin", 1);
		wall4->sendCommand("Change_Transform", Transform(vec3(0, -10, 22), glm::rotate(quat(1, 0, 0, 0), glm::radians(90.0f), vec3(0, 1, 0)), vec3(2)));
		*/
		Entity * wall5 = m_entityFactory.createEntity("Prop");
		wall5->sendCommand("Load_Model", string("Test\\wall.obj"));
		wall5->sendCommand("Change_Skin", 1);
		wall5->sendCommand("Change_Transform", Transform(vec3(20, -11, 0), glm::rotate(quat(1, 0, 0, 0), glm::radians(90.0f), vec3(0, 0, 1)), vec3(2)));

		Entity * wall6 = m_entityFactory.createEntity("Prop");
		wall6->sendCommand("Load_Model", string("Test\\wall.obj"));
		wall6->sendCommand("Change_Skin", 1);
		wall6->sendCommand("Change_Transform", Transform(vec3(-44, -10, -22), glm::rotate(quat(1, 0, 0, 0), glm::radians(90.0f), vec3(0, 1, 0)), vec3(2)));

		Entity * wall7 = m_entityFactory.createEntity("Prop");
		wall7->sendCommand("Load_Model", string("Test\\wall.obj"));
		wall7->sendCommand("Change_Skin", 1);
		wall7->sendCommand("Change_Transform", Transform(vec3(-44, -10, 22), glm::rotate(quat(1, 0, 0, 0), glm::radians(90.0f), vec3(0, 1, 0)), vec3(2)));

		Entity * sun = m_entityFactory.createEntity("Sun");
		sun->sendCommand("Change_Light_Color", vec3(0.75, 0.75, 0.9));
		sun->sendCommand("Change_Light_Intensity", 8.0f); // OLD INTENSITY WAS 8.0
		sun->sendCommand("Change_Transform", Transform(glm::rotate(quat(0.153046, -0.690346, 0.690346, 0.153046), glm::radians(45.0f), vec3(0, 0, 1))));

		/*auto point = m_entityFactory.createEntity("PointLight_Cheap");
		point->sendCommand("Change_Light_Color", vec3(0, 0, 1.0));
		point->sendCommand("Change_Light_Intensity", 15.0f);
		point->sendCommand("Change_Light_Radius", 10.0f);
		point->sendCommand("Change_Transform", Transform(vec3(0,0,0)));*/


		Entity * ref2 = m_entityFactory.createEntity("Reflector");
		ref2->sendCommand("Change_Transform", Transform(vec3(44, 15, 0), quat(1, 0, 0, 0), vec3(21)));

		Entity * ref = m_entityFactory.createEntity("Reflector");
		ref->sendCommand("Change_Transform", Transform(vec3(0, 15, 0), quat(1, 0, 0, 0), vec3(21)));

		/*auto spot = m_entityFactory.createEntity("PointLight");
		spot->sendCommand("Change_Light_Color", vec3(1));
		spot->sendCommand("Change_Light_Intensity", 15.0f);
		spot->sendCommand("Change_Light_Radius", 10.0f);
		spot->sendCommand("Change_Light_Cutoff", 45.0f);
		//spot->sendCommand("Change_Transform", Transform(vec3(-40, 0, 0), quat(1, 0, 0, 0)));*/

		Entity * h = m_entityFactory.createEntity("Prop");
		h->sendCommand("Load_Model", string("Test\\ref_test.obj"));
		h->sendCommand("Change_Transform", Transform(vec3(015, 0, -18), quat(1, 0, 0, 0), vec3(1)));

		Entity * m1 = m_entityFactory.createEntity("Prop");
		m1->sendCommand("Load_Model", string("Test\\AnimationTest.fbx"));
		m1->sendCommand("Change_Transform", Transform(vec3(-5, 0, 0)));

		Entity * m2 = m_entityFactory.createEntity("Prop");
		m2->sendCommand("Load_Model", string("Test\\AnimationTest.fbx"));
		m2->sendCommand("Change_Transform", Transform(vec3(5, 0, 0), glm::rotate(quat(1, 0, 0, 0), glm::radians(90.0f), vec3(0, 1, 0))));

		Entity * m3 = m_entityFactory.createEntity("Prop");
		m3->sendCommand("Load_Model", string("Test\\AnimationTest.fbx"));
		m3->sendCommand("Change_Transform", Transform(vec3(18, 0, -5)));

		Entity * m4 = m_entityFactory.createEntity("Prop");
		m4->sendCommand("Load_Model", string("Test\\AnimationTest.fbx"));
		m4->sendCommand("Change_Transform", Transform(vec3(0, 0, 5), glm::rotate(quat(1, 0, 0, 0), glm::radians(90.0f), vec3(0, 1, 0))));

		Entity * m5 = m_entityFactory.createEntity("Prop");
		m5->sendCommand("Load_Model", string("Test\\AnimationTest.fbx"));
		m5->sendCommand("Change_Transform", Transform(vec3(0, -5, 0), glm::rotate(quat(1, 0, 0, 0), glm::radians(90.0f), vec3(0, 1, 1))));

		Entity * m6 = m_entityFactory.createEntity("Prop");
		m6->sendCommand("Load_Model", string("Test\\AnimationTest.fbx"));
		m6->sendCommand("Change_Transform", Transform(vec3(-30, 0, -5)));

		Entity * m7 = m_entityFactory.createEntity("Prop");
		m7->sendCommand("Load_Model", string("Test\\AnimationTest.fbx"));
		m7->sendCommand("Change_Transform", Transform(vec3(-30, 0, 5)));		

		temp_loaded = true;
		m_loaded = false;
		m_worldChanged = true;

		//unloadWorld();
	}
}

void System_World::unloadWorld()
{
	lock_guard<shared_mutex> state_writeGuard(m_stateLock);
	m_worldChanged = true;
	m_loaded = false;

	m_entityFactory.flush();
	m_componentFactory.flush();

	lock_guard<shared_mutex> view_writeGuard(m_viewerLock);
	m_viewers.clear();
}

void System_World::checkWorld()
{
	if (m_worldChanged && !m_loaded) {
		for each (const auto pair in m_entityFactory.getEntities()) 
			for each (auto entity in pair.second) 
				if (!entity->isLoaded())
					return;		
		
		m_loaded = true;
		m_worldChanged = false;

		for each (bool * notifyee in m_loadNotifiers) 
			*notifyee = true;		
	}
}
