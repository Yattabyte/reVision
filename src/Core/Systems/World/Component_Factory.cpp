#include "Systems\World\Component_Factory.h"
#include "Systems\World\ECSmessage.h"
#include "Systems\World\ECSmessanger.h"
#include "Entities\Components\Anim_Model_Component.h"
#include "Entities\Components\Light_Directional_Component.h"

Component_Factory::~Component_Factory()
{
}

Component_Factory::Component_Factory() 
{
	m_Initialized = false;
}

void Component_Factory::Initialize(Engine_Package *enginePackage, ECSmessanger *ecsMessanger)
{
	if (!m_Initialized) {
		m_enginePackage = enginePackage;
		m_ECSmessanger = ecsMessanger;

		m_creatorMap.insert(pair<char*, ComponentCreator*>("Anim_Model", new Anim_Model_Creator(m_ECSmessanger)));
		m_creatorMap.insert(pair<char*, ComponentCreator*>("Light_Directional", new Light_Directional_Creator(m_ECSmessanger)));

		m_Initialized = true;
	}
}

ECShandle Component_Factory::CreateComponent(char * type, const ECShandle & parent_ID)
{
	unique_lock<shared_mutex> write_lock(m_dataLock);

	m_levelComponents.insert(pair<char*, vector<Component*>>(type, vector<Component*>()));
	Component *component;
	unsigned int spot;

	if ((m_freeSpots.find(type) != m_freeSpots.end()) && m_freeSpots[type].size()) {
		spot = m_freeSpots[type].front();
		m_levelComponents[type][spot] = nullptr;
		m_freeSpots[type].pop_front();
	}
	else {
		spot = m_levelComponents[type].size();
		m_levelComponents[type].push_back(nullptr);
	}

	component = m_creatorMap[type]->Create(ECShandle(type, spot), parent_ID, m_enginePackage);
	m_levelComponents[type][spot] = component;
	return ECShandle(type, spot);
}

void Component_Factory::DeleteComponent(const ECShandle & id)
{
	unique_lock<shared_mutex> write_lock(m_dataLock);

	if (m_levelComponents.find(id.first) == m_levelComponents.end())
		return;
	if (m_levelComponents[id.first].size() <= id.second)
		return;
	m_creatorMap[id.first]->Destroy(m_levelComponents.at(id.first).at(id.second));
	m_freeSpots.insert(pair<char*, deque<unsigned int>>(id.first, deque<unsigned int>()));
	m_freeSpots[id.first].push_back(id.second);
}

Component * Component_Factory::GetComponent(const ECShandle & id)
{
	shared_lock<shared_mutex> read_lock(m_dataLock);

	if (m_levelComponents.find(id.first) == m_levelComponents.end())
		return nullptr;
	return m_levelComponents[id.first][id.second];
}

vector<Component*>& Component_Factory::GetComponentsByType(char * type)
{
	shared_lock<shared_mutex> read_lock(m_dataLock);

	if (m_levelComponents.find(type) == m_levelComponents.end())
		return vector<Component*>();
	return m_levelComponents[type];
}

void Component_Factory::Flush()
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

shared_mutex & Component_Factory::GetDataLock()
{
	return m_dataLock;
}
