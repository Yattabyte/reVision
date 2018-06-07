#include "Systems\World\ECS\Component_Factory.h"
#include "Systems\World\ECS\Components\Anim_Model_Component.h"
#include "Systems\World\ECS\Components\Static_Model_Component.h"
#include "Systems\World\ECS\Components\Light_Directional_Component.h"
#include "Systems\World\ECS\Components\Light_Directional_Cheap_Component.h"
#include "Systems\World\ECS\Components\Light_Spot_Component.h"
#include "Systems\World\ECS\Components\Light_Spot_Cheap_Component.h"
#include "Systems\World\ECS\Components\Light_Point_Component.h"
#include "Systems\World\ECS\Components\Light_Point_Cheap_Component.h"
#include "Systems\World\ECS\Components\Reflector_Component.h"


Component_Factory::~Component_Factory()
{
}

Component_Factory::Component_Factory() 
{
	m_Initialized = false;
}

void Component_Factory::initialize(EnginePackage * enginePackage)
{
	if (!m_Initialized) {
		m_enginePackage = enginePackage;

		m_creatorMap["Anim_Model"] = new Anim_Model_Creator();
		m_creatorMap["Static_Model"] = new Static_Model_Creator();
		m_creatorMap["Light_Directional"] = new Light_Directional_Creator();
		m_creatorMap["Light_Directional_Cheap"] = new Light_Directional_Cheap_Creator();
		m_creatorMap["Light_Spot"] = new Light_Spot_Creator();
		m_creatorMap["Light_Spot_Cheap"] = new Light_Spot_Cheap_Creator();
		m_creatorMap["Light_Point"] = new Light_Point_Creator();
		m_creatorMap["Light_Point_Cheap"] = new Light_Point_Cheap_Creator();
		m_creatorMap["Reflector"] = new Reflector_Creator();

		m_Initialized = true;
	}
}

Component * Component_Factory::createComponent(const char * type)
{
	Component * component = m_creatorMap[type]->create(m_enginePackage);

	unique_lock<shared_mutex> write_lock(m_dataLock);
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
	unique_lock<shared_mutex> write_lock(m_dataLock);
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
		}
	}
}

const vector<Component*>& Component_Factory::getComponentsByType(const char * type)
{
	shared_lock<shared_mutex> read_lock(m_dataLock);
	return m_levelComponents[type];
}

void Component_Factory::flush()
{
	unique_lock<shared_mutex> write_lock(m_dataLock);

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
	shared_lock<shared_mutex> read_lock(m_dataLock);
	return m_levelComponents.find(key);
}

shared_mutex & Component_Factory::getDataLock()
{
	return m_dataLock;
}
