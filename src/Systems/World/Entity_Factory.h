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
class ECSmessenger;
class Component_Factory;


/**
 * A utility that handles the creation and storage of all level entities
 **/
class DT_ENGINE_API Entity_Factory
{
public:
	// (de)Constructors
	/** Destroy the factory. */
	~Entity_Factory();
	/** Construct the factory.
	 * @param	ecsMessenger		pointer to the entity-component messenger system 
	 * @param	componentFactory	pointer to the component factory */
	Entity_Factory(ECSmessenger * ecsMessenger, Component_Factory * componentFactory);


	// Public Methods
	/** Creates an entity of the supplied type and returns its handle.
	 * @param	type				the type-name of the entity to create
	 * @return						the ECShandle of the entity created */
	ECShandle CreateEntity(char * type);
	/** Delete the entity of the given handle.
	 * @param	id					the handle of the entity to delete */
	void DeleteEntity(const ECShandle & id);
	/** Retrieve the actual entity that matches the supplied ID.
	 * @param	id					the handle of the entity to retrieve
	 * @return						the entity who matches the handle provided */
	Entity * GetEntity(const ECShandle & id);
	/** Retrieves an array of entities that match the category specified.
	 * @param	type				the type-name of the entity list to retrieve
	 * @return						the list of entities that match the type provided */
	vector<Entity*> &GetEntitiesByType(char * type);
	/** Removes all entities from the system. */
	void Flush();


private:
	// Private Attributes
	map<char*, vector<Entity*>, cmp_str> m_levelEntities;
	map<char*, deque<unsigned int>> m_freeSpots;
	map<char*, EntityCreator*, cmp_str> m_creatorMap;
	ECSmessenger *m_ECSmessenger;
	Component_Factory *m_componentFactory;
};

#endif // ENTITY_FACTORY