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

System_World::System_World()
{
	m_loaded = false;
	m_worldChanged = false;
}

void System_World::initialize(Engine * engine)
{
	if (!m_Initialized) {
		m_engine = engine;

		m_Initialized = true;
	}
}

void System_World::update(const float & deltaTime)
{
	checkWorld();
	loadWorld();
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

	for each (const auto &type in std::vector<const char *>{ Model_Static_C::GetName(), Model_Animated_C::GetName(), Light_Directional_C::GetName(), Light_Directional_Cheap_C::GetName(), Light_Spot_C::GetName(), Light_Spot_Cheap_C::GetName(), Light_Point_C::GetName(), Light_Point_Cheap_C::GetName(), Reflector_C::GetName() }) {
		std::vector<Component*> visible_components;
		
		for each (auto component in m_engine->getECS().getSpecificComponents<Component>(type))
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
		ECS & ecs = m_engine->getECS();
		Component * hills = ecs.createComponent<Model_Animated_C>(
			std::string("Test\\hills.obj"), 
			0u, 
			-1, 
			Transform(glm::vec3(0, -7.5, 10), glm::quat(1, 0, 0, 0), glm::vec3(30))
		);
		Component * wall1 = ecs.createComponent<Model_Animated_C>(
			std::string("Test\\wall.obj"),
			0u,
			-1,
			Transform(glm::vec3(-22, -10, 0), glm::quat(1, 0, 0, 0), glm::vec3(2))
		);
		Component * wall2 = ecs.createComponent<Model_Animated_C>(
			std::string("Test\\wall.obj"),
			2u,
			-1,
			Transform(glm::vec3(22, -10, 0), glm::quat(1, 0, 0, 0), glm::vec3(2))
		);
		Component * wall3 = ecs.createComponent<Model_Animated_C>(
			std::string("Test\\wall.obj"),
			1u,
			-1,
			Transform(glm::vec3(0, -10, -22), glm::rotate(glm::quat(1, 0, 0, 0), glm::radians(90.0f), glm::vec3(0, 1, 0)), glm::vec3(2))
		);
		Component * wall5 = ecs.createComponent<Model_Animated_C>(
			std::string("Test\\wall.obj"),
			1u,
			-1,
			Transform(glm::vec3(20, -11, 0), glm::rotate(glm::quat(1, 0, 0, 0), glm::radians(90.0f), glm::vec3(0, 0, 1)), glm::vec3(2))
		);
		Component * wall6 = ecs.createComponent<Model_Animated_C>(
			std::string("Test\\wall.obj"),
			1u,
			-1,
			Transform(glm::vec3(-44, -10, -22), glm::rotate(glm::quat(1, 0, 0, 0), glm::radians(90.0f), glm::vec3(0, 1, 0)), glm::vec3(2))
		);
		Component * wall7 = ecs.createComponent<Model_Animated_C>(
			std::string("Test\\wall.obj"),
			1u,
			-1,
			Transform(glm::vec3(-44, -10, 22), glm::rotate(glm::quat(1, 0, 0, 0), glm::radians(90.0f), glm::vec3(0, 1, 0)), glm::vec3(2))
		);
		Component * h = ecs.createComponent<Model_Animated_C>(
			std::string("Test\\ref_test.obj"),
			0u,
			-1,
			Transform(glm::vec3(015, 0, -18), glm::quat(1, 0, 0, 0), glm::vec3(1))
		);
		Component * m1 = ecs.createComponent<Model_Animated_C>(
			std::string("Test\\AnimationTest.fbx"),
			1u,
			1,
			Transform(glm::vec3(-5, 0, 0))
		);
		Component * m2 = ecs.createComponent<Model_Animated_C>(
			std::string("Test\\AnimationTest.fbx"),
			0u,
			-1,
			Transform(glm::vec3(5, 0, 0), glm::rotate(glm::quat(1, 0, 0, 0), glm::radians(90.0f), glm::vec3(0, 1, 0)))
		);
		Component * m3 = ecs.createComponent<Model_Animated_C>(
			std::string("Test\\AnimationTest.fbx"),
			0u,
			-1,
			Transform(glm::vec3(18, 0, -5))
		);
		Component * m4 = ecs.createComponent<Model_Animated_C>(
			std::string("Test\\AnimationTest.fbx"),
			0u,
			-1,
			Transform(glm::vec3(0, 0, 5), glm::rotate(glm::quat(1, 0, 0, 0), glm::radians(90.0f), glm::vec3(0, 1, 0)))
		);
		Component * m5 = ecs.createComponent<Model_Animated_C>(
			std::string("Test\\AnimationTest.fbx"),
			0u,
			-1,
			Transform(glm::vec3(0, -5, 0), glm::rotate(glm::quat(1, 0, 0, 0), glm::radians(90.0f), glm::vec3(0, 1, 1)))
		);
		Component * m6 = ecs.createComponent<Model_Animated_C>(
			std::string("Test\\AnimationTest.fbx"),
			0u,
			-1,
			Transform(glm::vec3(-30, 0, -5))
		);
		Component * m7 = ecs.createComponent<Model_Animated_C>(
			std::string("Test\\AnimationTest.fbx"),
			0u,
			-1,
			Transform(glm::vec3(-30, 0, 5))
		);			
		Component * sun = ecs.createComponent<Light_Directional_C>(
			glm::vec3(0.75, 0.75, 0.9),
			8.0f,
			Transform(glm::rotate(glm::quat(0.153046, -0.690346, 0.690346, 0.153046), glm::radians(45.0f), glm::vec3(0, 0, 1)))
		);
		Component * ref2 = ecs.createComponent<Reflector_C>(
			Transform(glm::vec3(44, 15, 0), glm::quat(1, 0, 0, 0), glm::vec3(21))
		);
		Component * ref = ecs.createComponent<Reflector_C>(
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

	m_engine->getECS().flush();

	std::lock_guard<std::shared_mutex> view_writeGuard(m_viewerLock);
	m_viewers.clear();
}

void System_World::checkWorld()
{
	if (m_worldChanged && !m_loaded) {
		for each (const auto pair in m_engine->getECS().getComponents()) 
			for each (auto component in pair.second) 
				if (!component->isLoaded())
					return;		
		
		m_loaded = true;
		m_worldChanged = false;

		for each (bool * notifyee in m_loadNotifiers) 
			*notifyee = true;		
	}
}
