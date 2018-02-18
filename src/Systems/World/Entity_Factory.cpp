#include "Systems\World\Entity_Factory.h"
#include "Systems\World\ECSmessage.h"
#include "Systems\World\ECSmessenger.h"
#include "Entities\Prop.h"
#include "Entities\Sun.h"
#include "Entities\Light.h"


Entity_Factory::~Entity_Factory()
{
}

Entity_Factory::Entity_Factory(ECSmessenger *ecsMessenger, Component_Factory *componentFactory) : 
	m_ECSmessenger(ecsMessenger),
	m_componentFactory(componentFactory)
{
	m_creatorMap.insert(pair<char*, EntityCreator*>("Prop", new PropCreator()));
	m_creatorMap.insert(pair<char*, EntityCreator*>("Sun", new SunCreator()));
	m_creatorMap.insert(pair<char*, EntityCreator*>("SpotLight", new SpotLightCreator()));
	m_creatorMap.insert(pair<char*, EntityCreator*>("PointLight", new PointLightCreator()));
}

ECShandle Entity_Factory::CreateEntity(char * type)
{
	m_levelEntities.insert(pair<char*, vector<Entity*>>(type, vector<Entity*>()));
	Entity *entity;
	unsigned int spot;

	if ((m_freeSpots.find(type) != m_freeSpots.end()) && m_freeSpots[type].size()) {
		spot = m_freeSpots[type].front();
		m_levelEntities[type][spot] = nullptr;
		m_freeSpots[type].pop_front();
	}
	else {
		spot = m_levelEntities[type].size();
		m_levelEntities[type].push_back(nullptr);
	}

	entity = m_creatorMap[type]->create(ECShandle(type, spot), m_ECSmessenger, m_componentFactory);
	m_levelEntities[type][spot] = entity;
	return ECShandle(type, spot);
}

void Entity_Factory::DeleteEntity(const ECShandle & id)
{
	if (m_levelEntities.find(id.first) == m_levelEntities.end())
		return;
	if (m_levelEntities[id.first].size() <= id.second)
		return;
	m_creatorMap[id.first]->destroy(m_levelEntities.at(id.first).at(id.second));
	m_freeSpots.insert(pair<char*, deque<unsigned int>>(id.first, deque<unsigned int>()));
	m_freeSpots[id.first].push_back(id.second);
}

Entity * Entity_Factory::GetEntity(const ECShandle & id)
{
	if (m_levelEntities.find(id.first) == m_levelEntities.end())
		return nullptr;
	return m_levelEntities[id.first][id.second];
}

vector<Entity*>& Entity_Factory::GetEntitiesByType(char * type)
{
	if (m_levelEntities.find(type) == m_levelEntities.end())
		return vector<Entity*>();
	return m_levelEntities[type];
}

void Entity_Factory::Flush()
{
	for each (auto pair in m_levelEntities) {
		for each (auto *entity in pair.second) {
			m_creatorMap[pair.first]->destroy(entity);
		}
	}
	m_levelEntities.clear();
	m_freeSpots.clear();
}
