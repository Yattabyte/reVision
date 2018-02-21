#pragma once
#ifndef ENTITY_FACTORY
#define ENTITY_FACTORY
#ifdef	ENGINE_EXPORT
#define DT_ENGINE_API __declspec(dllexport)
#else
#define	DT_ENGINE_API __declspec(dllimport)
#endif

#include "Systems\World\ECS\Entities\Entity.h"
#include "Utilities\MappedChar.h"
#include <deque>

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
	ECShandle createEntity(const char * type);
	/** Delete the entity of the given handle.
	 * @param	id					the handle of the entity to delete */
	void deleteEntity(const ECShandle & id);
	/** Retrieve the actual entity that matches the supplied ID.
	 * @param	id					the handle of the entity to retrieve
	 * @return						the entity who matches the handle provided */
	Entity * getEntity(const ECShandle & id);
	/** Retrieves an array of entities that match the category specified.
	 * @brief						Guaranteed to return at least a zero-length vector. Types that don't exist are created.
	 * @param	type				the type-name of the entity list to retrieve
	 * @return						the list of entities that match the type provided */
	vector<Entity*> & getEntitiesByType(const char * type);
	/** Removes all entities from the system. */
	void flush();


private:
	// Private Attributes
	VectorMap<Entity*> m_levelEntities; 
	MappedChar<deque<unsigned int>> m_freeSpots;
	MappedChar<EntityCreator*> m_creatorMap;
	ECSmessenger *m_ECSmessenger;
	Component_Factory *m_componentFactory;
};

#endif // ENTITY_FACTORY