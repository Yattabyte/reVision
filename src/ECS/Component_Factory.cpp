#include "ECS\Component_Factory.h"
#include "ECS\Components\Model_Animated.h"
#include "ECS\Components\Model_Static.h"
#include "ECS\Components\Light_Directional.h"
#include "ECS\Components\Light_Directional_Cheap.h"
#include "ECS\Components\Light_Spot.h"
#include "ECS\Components\Light_Spot_Cheap.h"
#include "ECS\Components\Light_Point.h"
#include "ECS\Components\Light_Point_Cheap.h"
#include "ECS\Components\Reflector.h"
#include "Engine.h"


Component_Factory::~Component_Factory()
{
}

Component_Factory::Component_Factory() 
{
	m_Initialized = false;
}

void Component_Factory::initialize(Engine * engine)
{
	if (!m_Initialized) {
		m_engine = engine;

		m_creatorMap["Anim_Model"] = new Component_Creator<Model_Animated_C>();
		m_creatorMap["Static_Model"] = new Component_Creator<Model_Static_C>();
		m_creatorMap["Light_Directional"] = new Component_Creator<Light_Directional_C>();
		m_creatorMap["Light_Directional_Cheap"] = new Component_Creator<Light_Directional_Cheap_C>();
		m_creatorMap["Light_Spot"] = new Component_Creator<Light_Spot_C>();
		m_creatorMap["Light_Spot_Cheap"] = new Component_Creator<Light_Spot_Cheap_C>();
		m_creatorMap["Light_Point"] = new Component_Creator<Light_Point_C>();
		m_creatorMap["Light_Point_Cheap"] = new Component_Creator<Light_Point_Cheap_C>();
		m_creatorMap["Reflector"] = new Component_Creator<Reflector_C>();

		m_Initialized = true;
	}
}

Component * Component_Factory::createComponent(const char * type)
{
	if (!m_creatorMap.find(type)) {
		m_engine->reportError(MessageManager::OTHER_ERROR, "Component type: '" + std::string(type) + "' not found");
		return nullptr;
	}

	Component * component = m_creatorMap[type]->create(m_engine);

	std::unique_lock<std::shared_mutex> write_lock(m_dataLock);
	m_levelComponents.insert(type);

	if (m_freeSpots.find(type) && m_freeSpots[type].size()) {
		m_levelComponents[type][m_freeSpots[type].front()] = component;
		m_freeSpots[type].pop_front();
	}
	else 
		m_levelComponents[type].push_back(component);	

	return component;
}

void Component_Factory::deleteComponent(Component * component)
{
	std::unique_lock<std::shared_mutex> write_lock(m_dataLock);
	const char * type = component->getName();
	if (!component || !m_levelComponents.find(type) || !m_creatorMap.find(type))
		return;
	for (auto itt = m_levelComponents[type].begin(); itt != m_levelComponents[type].end(); ++itt) {
		Component * lvlComponent = *itt;
		if (lvlComponent == component) {
			unsigned int oldIndex = std::distance(m_levelComponents[type].begin(), itt);
			m_creatorMap[type]->destroy(component);
			m_freeSpots.insert(type);
			m_freeSpots[type].push_back(oldIndex);
			m_levelComponents[type].erase(itt, itt+1);
			return;
		}
	}
}

VectorMap<Component*>& Component_Factory::getComponents()
{
	return m_levelComponents;
}

const std::vector<Component*>& Component_Factory::getComponentsByType(const char * type)
{
	std::shared_lock<std::shared_mutex> read_lock(m_dataLock);
	return m_levelComponents[type];
}

void Component_Factory::flush()
{
	std::unique_lock<std::shared_mutex> write_lock(m_dataLock);

	for each (auto pair in m_levelComponents) {
		for each (auto *component in pair.second) {
			m_creatorMap[pair.first]->destroy(component);
		}
	}
	m_levelComponents.clear();
	m_freeSpots.clear();
}

bool Component_Factory::find(const char * key) const
{
	std::shared_lock<std::shared_mutex> read_lock(m_dataLock);
	return m_levelComponents.find(key);
}

std::shared_mutex & Component_Factory::getDataLock()
{
	return m_dataLock;
}
