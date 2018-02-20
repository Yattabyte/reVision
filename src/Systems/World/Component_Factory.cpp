#include "Systems\World\Component_Factory.h"
#include "Systems\World\ECSmessage.h"
#include "Systems\World\ECSmessenger.h"
#include "Entities\Components\Anim_Model_Component.h"
#include "Entities\Components\Light_Directional_Component.h"
#include "Entities\Components\Light_Spot_Component.h"
#include "Entities\Components\Light_Point_Component.h"


Component_Factory::~Component_Factory()
{
}

Component_Factory::Component_Factory() 
{
	m_Initialized = false;
}

void Component_Factory::initialize(EnginePackage * enginePackage, ECSmessenger * ecsMessenger)
{
	if (!m_Initialized) {
		m_enginePackage = enginePackage;
		m_ECSmessenger = ecsMessenger;

		m_creatorMap["Anim_Model"] = new Anim_Model_Creator(m_ECSmessenger);
		m_creatorMap["Light_Directional"] = new Light_Directional_Creator(m_ECSmessenger);
		m_creatorMap["Light_Spot"] = new Light_Spot_Creator(m_ECSmessenger);
		m_creatorMap["Light_Point"] = new Light_Point_Creator(m_ECSmessenger);

		m_Initialized = true;
	}
}

ECShandle Component_Factory::createComponent(const char * type, const ECShandle & parent_ID)
{
	unique_lock<shared_mutex> write_lock(m_dataLock);

	m_levelComponents.insert(type);
	unsigned int spot;

	// Component creation can be lengthy, and may create more components
	// Need to unlock mutex before creating and re-lock before adding the component to the list
	if (m_freeSpots.find(type) && m_freeSpots[type].size()) {
		spot = m_freeSpots[type].front();
		m_freeSpots[type].pop_front();
		write_lock.unlock();
		write_lock.release();
		Component *component = m_creatorMap[type]->Create(ECShandle(type, spot), parent_ID, m_enginePackage);
		unique_lock<shared_mutex> write_lock2(m_dataLock);
		m_levelComponents[type][spot] = component;
	}
	else {
		spot = m_levelComponents[type].size();
		write_lock.unlock();
		write_lock.release();
		Component *component = m_creatorMap[type]->Create(ECShandle(type, spot), parent_ID, m_enginePackage);
		unique_lock<shared_mutex> write_lock2(m_dataLock);
		m_levelComponents[type].push_back(component);
	}
	return ECShandle(type, spot);
}

void Component_Factory::deleteComponent(const ECShandle & id)
{
	unique_lock<shared_mutex> write_lock(m_dataLock);

	if (!m_levelComponents.find(id.first))
		return;
	if (m_levelComponents[id.first].size() <= id.second)
		return;
	m_creatorMap[id.first]->Destroy(m_levelComponents.at(id.first).at(id.second));
	m_freeSpots.insert(id.first);
	m_freeSpots[id.first].push_back(id.second);
}

Component * Component_Factory::getComponent(const ECShandle & id)
{
	shared_lock<shared_mutex> read_lock(m_dataLock);

	if (!m_levelComponents.find(id.first))
		return nullptr;
	return m_levelComponents[id.first][id.second];
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
			m_creatorMap[pair.first]->Destroy(component);
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
