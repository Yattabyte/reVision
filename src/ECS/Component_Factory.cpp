#include "ECS\Component_Factory.h"
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
		m_Initialized = true;
	}
}

void Component_Factory::deleteComponent(Component * component)
{
	std::unique_lock<std::shared_mutex> write_lock(m_dataLock);
	const char * type = component->getName();
	if (!component || !m_levelComponents.find(type))
		return;
	for (auto itt = m_levelComponents[type].begin(); itt != m_levelComponents[type].end(); ++itt) {
		Component * lvlComponent = *itt;
		if (lvlComponent == component) {
			unsigned int oldIndex = std::distance(m_levelComponents[type].begin(), itt);
			delete component;
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

	for each (auto pair in m_levelComponents) 
		for each (auto *component in pair.second) 
			delete component;
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
