#pragma once
#ifndef ENTITY_FACTORY_H
#define ENTITY_FACTORY_H

#include "Systems\World\ECS\Entities\Entity.h"
#include "Utilities\MappedChar.h"
#include <deque>


using namespace std;
class Component_Factory;

/**
 * A utility that handles the creation and storage of all level entities
 **/
class Entity_Factory
{
public:
	// (de)Constructors
	/** Destroy the factory. */
	~Entity_Factory();
	/** Construct the factory.
	 * @param	componentFactory	pointer to the component factory */
	Entity_Factory(Component_Factory * componentFactory);


	// Public Methods
	/** Creates an entity of the supplied type and returns its handle.
	 * @param	type				the type-name of the entity to create
	 * @return						the newly created entity */
	Entity * createEntity(const char * type);
	/** Delete the entity of the given handle.
	 * @param	id					the handle of the entity to delete */
	void deleteEntity(const char * type, Entity * entity);
	/** Retrieves reference to the level entities vector-map
	 * @return						the entire level entity vector-map */
	VectorMap<Entity*> & getEntities();
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
};

#endif // ENTITY_FACTORY_H