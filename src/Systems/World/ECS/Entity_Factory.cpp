#include "Systems\World\ECS\Entity_Factory.h"
#include "Systems\World\ECS\Entities\Entity.h"
#include "Systems\World\ECS\Entities\Lights.h"
#include "Systems\World\ECS\Entities\Props.h"
#include "Systems\World\ECS\Entities\Reflector.h"
#include "Systems\World\ECS\Entities\Sun.h"
#include "Systems\World\ECS\Entities\Sun_Cheap.h"


Entity_Factory::~Entity_Factory()
{
}

Entity_Factory::Entity_Factory(Component_Factory *componentFactory) 
{
	m_creatorMap["Prop"] = new Creator_Prop(componentFactory);
	m_creatorMap["Prop_Static"] = new Creator_Prop_Static(componentFactory);
	m_creatorMap["SpotLight"] = new Creator_SpotLight(componentFactory);
	m_creatorMap["SpotLight_Cheap"] = new Creator_SpotLight_Cheap(componentFactory);
	m_creatorMap["PointLight"] = new Creator_PointLight(componentFactory);
	m_creatorMap["PointLight_Cheap"] = new Creator_PointLight_Cheap(componentFactory);
	m_creatorMap["Reflector"] = new Creator_Reflector(componentFactory);
	m_creatorMap["Sun"] = new Creator_Sun(componentFactory);
	m_creatorMap["Sun_Cheap"] = new Creator_Sun_Cheap(componentFactory);
}

Entity * Entity_Factory::createEntity(const char * type)
{
	Entity *entity = m_creatorMap[type]->create();

	m_levelEntities.insert(type);
	if (m_freeSpots.find(type) && m_freeSpots[type].size()) {
		m_levelEntities[type][m_freeSpots[type].front()] = entity;
		m_freeSpots[type].pop_front();
	}
	else 
		m_levelEntities[type].push_back(entity);	

	return entity;
}

void Entity_Factory::deleteEntity(const char * type, Entity * entity)
{
	if (!entity || !m_levelEntities.find(type) || !m_creatorMap.find(type))
		return;
	for (auto itt = m_levelEntities[type].begin(); itt != m_levelEntities[type].end(); ++itt) {
		Entity * lvlEntity = *itt;
		if (lvlEntity == entity) {
			unsigned int oldIndex = std::distance(m_levelEntities[type].begin(), itt);
			m_creatorMap[type]->destroy(entity);
			m_freeSpots.insert(type);
			m_freeSpots[type].push_back(oldIndex);
		}
	}
}

VectorMap<Entity*>& Entity_Factory::getEntities()
{
	return m_levelEntities;
}

vector<Entity*>& Entity_Factory::getEntitiesByType(const char * type)
{
	return m_levelEntities[type];
}

void Entity_Factory::flush()
{
	for each (auto pair in m_levelEntities)
		for each (auto *entity in pair.second)
			m_creatorMap[pair.first]->destroy(entity);	
	m_levelEntities.clear();
	m_freeSpots.clear();
}