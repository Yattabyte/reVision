#include "Systems\World\ECS\Entity_Factory.h"
#include "Systems\World\ECS\ECSmessage.h"
#include "Systems\World\ECS\ECSmessenger.h"
#include "Systems\World\ECS\Entities\Entity.h"
#include "Systems\World\ECS\Entities\Light.h"
#include "Systems\World\ECS\Entities\Prop.h"
#include "Systems\World\ECS\Entities\Reflector.h"
#include "Systems\World\ECS\Entities\Sun.h"


Entity_Factory::~Entity_Factory()
{
}

Entity_Factory::Entity_Factory(ECSmessenger *ecsMessenger, Component_Factory *componentFactory) : 
	m_ECSmessenger(ecsMessenger),
	m_componentFactory(componentFactory)
{
	m_creatorMap["SpotLight"] = new SpotLightCreator();
	m_creatorMap["PointLight"] = new PointLightCreator();
	m_creatorMap["Prop"] = new PropCreator();
	m_creatorMap["Reflector"] = new ReflectorCreator();
	m_creatorMap["Sun"] = new SunCreator();
}

ECShandle Entity_Factory::createEntity(const char * type)
{
	m_levelEntities.insert(type);
	Entity *entity;
	unsigned int spot;

	if (m_freeSpots.find(type) && m_freeSpots[type].size()) {
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

void Entity_Factory::deleteEntity(const ECShandle & id)
{
	if (!m_levelEntities.find(id.first))
		return;
	if (m_levelEntities[id.first].size() <= id.second)
		return;
	m_creatorMap[id.first]->destroy(m_levelEntities.at(id.first).at(id.second));
	m_freeSpots.insert(id.first);
	m_freeSpots[id.first].push_back(id.second);
}

Entity * Entity_Factory::getEntity(const ECShandle & id)
{
	if (!m_levelEntities.find(id.first))
		return nullptr;
	return m_levelEntities[id.first][id.second];
}

vector<Entity*>& Entity_Factory::getEntitiesByType(const char * type)
{
	return m_levelEntities[type];
}

void Entity_Factory::flush()
{
	for each (auto pair in m_levelEntities) {
		for each (auto *entity in pair.second) {
			m_creatorMap[pair.first]->destroy(entity);
		}
	}
	m_levelEntities.clear();
	m_freeSpots.clear();
}