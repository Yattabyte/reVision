/*
	Entity_Factory

	- Handles creation and storage for all level entities
*/



#pragma once
#ifndef ENTITY_FACTORY
#define ENTITY_FACTORY
#ifdef	ENGINE_EXPORT
#define DT_ENGINE_API __declspec(dllexport)
#else
#define	DT_ENGINE_API __declspec(dllimport)
#endif

#include "Entities\Entity.h"
#include <deque>
#include <map>
#include <vector>

using namespace std;

class ECSmessanger;
class Component_Factory;
class DT_ENGINE_API Entity_Factory
{
public:
	~Entity_Factory();
	Entity_Factory(ECSmessanger *ecsMessanger, Component_Factory *componentFactory);

	// Creates an entity of the supplied type and returns the handle
	ECShandle CreateEntity(char *type);
	// Delete the entity that matches the supplied ID
	void DeleteEntity(const ECShandle& id);
	// Retrieve the actual entity that matches the supplied ID
	Entity * GetEntity(const ECShandle& id);
	// Retrieve an array of entities that match the category specified
	vector<Entity*> &GetEntitiesByType(char *type);
	// Removes all entities from the system
	void Flush();

private:
	map<char*, vector<Entity*>, cmp_str> m_levelEntities;
	map<char*, deque<unsigned int>> m_freeSpots;
	map<char*, EntityCreator*, cmp_str> m_creatorMap;
	ECSmessanger *m_ECSmessanger;
	Component_Factory *m_componentFactory;
};

#endif // ENTITY_FACTORY