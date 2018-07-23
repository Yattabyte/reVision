#include "ECS\ECS.h"
#include "Engine.h"

#include "ECS\Components\Model_Animated.h"
#include "ECS\Components\Model_Static.h"
#include "ECS\Components\Light_Directional.h"
#include "ECS\Components\Light_Directional_Cheap.h"
#include "ECS\Components\Light_Spot.h"
#include "ECS\Components\Light_Spot_Cheap.h"
#include "ECS\Components\Light_Point.h"
#include "ECS\Components\Light_Point_Cheap.h"
#include "ECS\Components\Reflector.h"


ECS::~ECS()
{
}

ECS::ECS(Engine * engine) : m_engine(engine) 
{
	m_creatorMap[Model_Animated_C::GetName()] = new Component_Creator<Model_Animated_C>();
	m_creatorMap[Model_Static_C::GetName()] = new Component_Creator<Model_Static_C>();
	m_creatorMap[Light_Directional_C::GetName()] = new Component_Creator<Light_Directional_C>();
	m_creatorMap[Light_Directional_Cheap_C::GetName()] = new Component_Creator<Light_Directional_Cheap_C>();
	m_creatorMap[Light_Spot_C::GetName()] = new Component_Creator<Light_Spot_C>();
	m_creatorMap[Light_Spot_Cheap_C::GetName()] = new Component_Creator<Light_Spot_Cheap_C>();
	m_creatorMap[Light_Point_C::GetName()] = new Component_Creator<Light_Point_C>();
	m_creatorMap[Light_Point_Cheap_C::GetName()] = new Component_Creator<Light_Point_Cheap_C>();
	m_creatorMap[Reflector_C::GetName()] = new Component_Creator<Reflector_C>();
}

void ECS::deleteEntity(Entity * entity)
{
	std::unique_lock<std::shared_mutex> write_lock(m_dataLock);
	// Exit early if entity is null
	if (!entity)
		return;

	// Delete each of the entity's components from the level
	for each (auto pair in entity->m_components)
		for each (Component * component in pair.second)
			deleteComponent(component);

	// Remove the entity from the level
	m_levelEntities.erase(std::remove_if(begin(m_levelEntities), end(m_levelEntities), [entity](Entity * levelEntity) {
		return (levelEntity == entity);
	}), end(m_levelEntities));

	// Delete the entity
	delete entity;
}

Component * ECS::createComponent(const char * type, const ArgumentList & argumentList)
{
	if (!m_creatorMap.find(type))
		return nullptr;

	Component * component = ((Component_Creator_Base*)(m_creatorMap[type]))->create(m_engine, argumentList);

	std::unique_lock<std::shared_mutex> write_lock(m_dataLock);
	if (!m_levelComponents.find(type)) 
		m_levelComponents.insert((new std::string(type))->c_str());		
	
	m_levelComponents[type].push_back(component);

	return component;
}

void ECS::deleteComponent(Component * component)
{
	std::unique_lock<std::shared_mutex> write_lock(m_dataLock);
	// Exit early if component is null
	const char * type = component->GetName();
	if (!component || !m_levelComponents.find(type))
		return;
	
	// Remove the component from the level
	m_levelComponents[type].erase(std::remove_if(begin(m_levelComponents[type]), end(m_levelComponents[type]), [component](Component * levelComponent) {
		return (levelComponent == component);
	}), end(m_levelComponents[type]));

	// Delete the component
	delete component;
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

	for each (Entity * entity in m_levelEntities)
		delete entity;
	for each (auto pair in m_levelComponents) 
		for each (Component * component in pair.second) 
			delete component;

	m_levelEntities.clear();
	m_levelComponents.clear();
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