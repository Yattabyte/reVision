#include "Systems\World\World.h"
#include "Systems\World\Visibility_Token.h"
#include "Engine.h"
#include "ECS\ECS_DEFINES.h"
#include "Utilities\Transform.h"
#include <algorithm>

#include "ECS\Components\Model_Animated.h"
#include "ECS\Components\Model_Static.h"
#include "ECS\Components\Light_Directional.h"
#include "ECS\Components\Light_Directional_Cheap.h"
#include "ECS\Components\Light_Spot.h"
#include "ECS\Components\Light_Spot_Cheap.h"
#include "ECS\Components\Light_Point.h"
#include "ECS\Components\Light_Point_Cheap.h"
#include "ECS\Components\Reflector.h"

System_World::~System_World()
{
}

System_World::System_World() : 	
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
	std::shared_lock<std::shared_mutex> stateGuard(m_stateLock);
	//if (!m_worldChanged && m_loaded) {
		calcVisibility(*m_engine->getCamera());
		std::shared_lock<std::shared_mutex> viewerGuard(m_viewerLock);
		for each (auto &camera in m_viewers)
			calcVisibility(*camera);
	//}
}

void System_World::registerViewer(Camera * c)
{
	std::unique_lock<std::shared_mutex> writeGuard(m_viewerLock);
	m_viewers.push_back(c);
}

void System_World::unregisterViewer(Camera * c)
{
	std::unique_lock<std::shared_mutex> writeGuard(m_viewerLock);
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
	const glm::vec3 &eyePos = camBuffer.EyePosition;
	Visibility_Token vis_token;

	for each (const auto &type in std::vector<const char *>{ "Static_Model", "Anim_Model", "Light_Directional", "Light_Directional_Cheap", "Light_Spot", "Light_Spot_Cheap", "Light_Point", "Light_Point_Cheap", "Reflector" }) {
		std::vector<Component*> visible_components;
		
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
		Component * hills = m_componentFactory.createComponent<Model_Animated_C>("Anim_Model",
			std::string("Test\\hills.obj"), 
			0u, 
			-1, 
			Transform(glm::vec3(0, -7.5, 10), glm::quat(1, 0, 0, 0), glm::vec3(30))
		);
		Component * wall1 = m_componentFactory.createComponent<Model_Animated_C>("Anim_Model",
			std::string("Test\\wall.obj"),
			0u,
			-1,
			Transform(glm::vec3(-22, -10, 0), glm::quat(1, 0, 0, 0), glm::vec3(2))
		);
		Component * wall2 = m_componentFactory.createComponent<Model_Animated_C>("Anim_Model",
			std::string("Test\\wall.obj"),
			2u,
			-1,
			Transform(glm::vec3(22, -10, 0), glm::quat(1, 0, 0, 0), glm::vec3(2))
		);
		Component * wall3 = m_componentFactory.createComponent<Model_Animated_C>("Anim_Model",
			std::string("Test\\wall.obj"),
			1u,
			-1,
			Transform(glm::vec3(0, -10, -22), glm::rotate(glm::quat(1, 0, 0, 0), glm::radians(90.0f), glm::vec3(0, 1, 0)), glm::vec3(2))
		);
		Component * wall5 = m_componentFactory.createComponent<Model_Animated_C>("Anim_Model",
			std::string("Test\\wall.obj"),
			1u,
			-1,
			Transform(glm::vec3(20, -11, 0), glm::rotate(glm::quat(1, 0, 0, 0), glm::radians(90.0f), glm::vec3(0, 0, 1)), glm::vec3(2))
		);
		Component * wall6 = m_componentFactory.createComponent<Model_Animated_C>("Anim_Model",
			std::string("Test\\wall.obj"),
			1u,
			-1,
			Transform(glm::vec3(-44, -10, -22), glm::rotate(glm::quat(1, 0, 0, 0), glm::radians(90.0f), glm::vec3(0, 1, 0)), glm::vec3(2))
		);
		Component * wall7 = m_componentFactory.createComponent<Model_Animated_C>("Anim_Model",
			std::string("Test\\wall.obj"),
			1u,
			-1,
			Transform(glm::vec3(-44, -10, 22), glm::rotate(glm::quat(1, 0, 0, 0), glm::radians(90.0f), glm::vec3(0, 1, 0)), glm::vec3(2))
		);
		Component * h = m_componentFactory.createComponent<Model_Animated_C>("Anim_Model",
			std::string("Test\\ref_test.obj"),
			0u,
			-1,
			Transform(glm::vec3(015, 0, -18), glm::quat(1, 0, 0, 0), glm::vec3(1))
		);
		Component * m1 = m_componentFactory.createComponent<Model_Animated_C>("Anim_Model",
			std::string("Test\\AnimationTest.fbx"),
			1u,
			1,
			Transform(glm::vec3(-5, 0, 0))
		);
		Component * m2 = m_componentFactory.createComponent<Model_Animated_C>("Anim_Model",
			std::string("Test\\AnimationTest.fbx"),
			0u,
			-1,
			Transform(glm::vec3(5, 0, 0), glm::rotate(glm::quat(1, 0, 0, 0), glm::radians(90.0f), glm::vec3(0, 1, 0)))
		);
		Component * m3 = m_componentFactory.createComponent<Model_Animated_C>("Anim_Model",
			std::string("Test\\AnimationTest.fbx"),
			0u,
			-1,
			Transform(glm::vec3(18, 0, -5))
		);
		Component * m4 = m_componentFactory.createComponent<Model_Animated_C>("Anim_Model",
			std::string("Test\\AnimationTest.fbx"),
			0u,
			-1,
			Transform(glm::vec3(0, 0, 5), glm::rotate(glm::quat(1, 0, 0, 0), glm::radians(90.0f), glm::vec3(0, 1, 0)))
		);
		Component * m5 = m_componentFactory.createComponent<Model_Animated_C>("Anim_Model",
			std::string("Test\\AnimationTest.fbx"),
			0u,
			-1,
			Transform(glm::vec3(0, -5, 0), glm::rotate(glm::quat(1, 0, 0, 0), glm::radians(90.0f), glm::vec3(0, 1, 1)))
		);
		Component * m6 = m_componentFactory.createComponent<Model_Animated_C>("Anim_Model",
			std::string("Test\\AnimationTest.fbx"),
			0u,
			-1,
			Transform(glm::vec3(-30, 0, -5))
		);
		Component * m7 = m_componentFactory.createComponent<Model_Animated_C>("Anim_Model",
			std::string("Test\\AnimationTest.fbx"),
			0u,
			-1,
			Transform(glm::vec3(-30, 0, 5))
		);			
		Component * sun = m_componentFactory.createComponent<Light_Directional_C>("Light_Directional",
			glm::vec3(0.75, 0.75, 0.9),
			8.0f,
			Transform(glm::rotate(glm::quat(0.153046, -0.690346, 0.690346, 0.153046), glm::radians(45.0f), glm::vec3(0, 0, 1)))
		);
		Component * ref2 = m_componentFactory.createComponent<Reflector_C>("Reflector",
			Transform(glm::vec3(44, 15, 0), glm::quat(1, 0, 0, 0), glm::vec3(21))
		);
		Component * ref = m_componentFactory.createComponent<Reflector_C>("Reflector",
			Transform(glm::vec3(0, 15, 0), glm::quat(1, 0, 0, 0), glm::vec3(21))
		);


		temp_loaded = true;
		m_loaded = false;
		m_worldChanged = true;

		//unloadWorld();
	}
}

void System_World::unloadWorld()
{
	std::lock_guard<std::shared_mutex> state_writeGuard(m_stateLock);
	m_worldChanged = true;
	m_loaded = false;

	m_componentFactory.flush();

	std::lock_guard<std::shared_mutex> view_writeGuard(m_viewerLock);
	m_viewers.clear();
}

void System_World::checkWorld()
{
	if (m_worldChanged && !m_loaded) {
		for each (const auto pair in m_componentFactory.getComponents()) 
			for each (auto component in pair.second) 
				if (!component->isLoaded())
					return;		
		
		m_loaded = true;
		m_worldChanged = false;

		for each (bool * notifyee in m_loadNotifiers) 
			*notifyee = true;		
	}
}
