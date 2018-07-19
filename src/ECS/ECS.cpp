#include "ECS\ECS.h"
#include "Engine.h"


ECS::~ECS()
{
}

void ECS::deleteComponent(Component * component)
{
	std::unique_lock<std::shared_mutex> write_lock(m_dataLock);
	const char * type = component->GetName();
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

VectorMap<Component*>& ECS::getComponents()
{
	return m_levelComponents;
}

const std::vector<Component*>& ECS::getComponentsByType(const char * type)
{
	std::shared_lock<std::shared_mutex> read_lock(m_dataLock);
	return m_levelComponents[type];
}

void ECS::flush()
{
	std::unique_lock<std::shared_mutex> write_lock(m_dataLock);

	for each (auto pair in m_levelComponents) 
		for each (auto *component in pair.second) 
			delete component;
	m_levelComponents.clear();
	m_freeSpots.clear();
}

bool ECS::find(const char * key) const
{
	std::shared_lock<std::shared_mutex> read_lock(m_dataLock);
	return m_levelComponents.find(key);
}

std::shared_mutex & ECS::getDataLock()
{
	return m_dataLock;
}
