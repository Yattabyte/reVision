#pragma once
#ifndef ENTITY
#define ENTITY
#ifdef	ENGINE_EXPORT
#define DT_ENGINE_API __declspec(dllexport)
#else
#define	DT_ENGINE_API __declspec(dllimport)
#endif
#define GLEW_STATIC

#include "Systems\World\ECSmessage.h"
#include "Systems\World\ECSdefines.h"
#include "GL\glew.h"
#include <map>
#include <vector>

class ECSmessenger;
class Component_Factory;
class EntityCreator;
class Component;


/**
 * A super object composed of components.\n
 * Because of this, we don't re-implement entities, instead we re-implement entity-creators to dish out the components needed per entity.
 **/
class DT_ENGINE_API Entity
{
public:
	// Public Methods
	/** Generates a new component for this entity of the supplied type.
	 * @param	type	const char array name of the component type to add */
	void addComponent(char * type);
	/** Returns a component if it exists from this entity, using the supplied handle.
	 * @param	id	the handle of the component to return
	 * @return	the component which matches the handle supplied */
	Component* getComponent(const ECShandle & id);	
	/** Propagate a message onto this entity's components.
	 * @param	message	the message to send */
	void receiveMessage(const ECSmessage & message);
	

protected:
	// (de)Constructors
	/** Virtual Destructor. */
	virtual ~Entity();
	/** Constructor with handle identifier into entity factory. */
	Entity(const ECShandle & id) : m_ID(id) {};


	// Protected Attributes
	ECShandle m_ID;
	std::map<char *, std::vector<unsigned int>, cmp_str> m_component_handles;
	ECSmessenger *m_ECSmessenger;
	Component_Factory *m_componentFactory;
	friend class EntityCreator;
};

/**
 * An interface to direct the creation of specific entities.
 **/
class DT_ENGINE_API EntityCreator
{
public:
	/** Virtual Destructor. */
	virtual ~EntityCreator(void) {};

	/** Destroy the entity.
	 * @param	entity	the entity to delete */
	virtual void destroy(Entity * entity) { delete entity; };

	/** Creates an entity
	 * @param	id	the handle identifier for the entity
	 * @param	ecsMessenger	pointer to the messenger system allowing communication between entities and components
	 * @param	componentFactory	pointer to the component factory to allow creation of specific components
	 * @return	the entity created */
	virtual Entity* create(const ECShandle & id, ECSmessenger * ecsMessenger, Component_Factory * componentFactory) { 
		Entity *entity = new Entity(id);
		entity->m_ECSmessenger = ecsMessenger;
		entity->m_componentFactory = componentFactory;
		return entity;
	};
};

#endif // ENTITY