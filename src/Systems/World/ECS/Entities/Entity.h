#pragma once
#ifndef ENTITY
#define ENTITY
#ifdef	ENGINE_EXE_EXPORT
#define DT_ENGINE_API 
#elif	ENGINE_DLL_EXPORT 
#define DT_ENGINE_API __declspec(dllexport)
#else
#define	DT_ENGINE_API __declspec(dllimport)
#endif
#define GLEW_STATIC

#include "Systems\World\ECS\ECSmessage.h"
#include "Utilities\MappedChar.h"
#include "GL\glew.h"
#include <functional>

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
	template <typename DATA_TYPE>
	void sendCommand(const char * command, const DATA_TYPE & obj) {
		// Run the command if it exists, and pass it the payload
		if (m_commandMap.find(command))
			m_commandMap[command](ECS_Command(obj));
	}
	/** Returns true if this component is loaded, false otherwise. */
	virtual bool isLoaded() const { return this ? true : false; }
	

protected:
	// (de)Constructors
	/** Virtual Destructor. */
	virtual ~Entity() {};
	/** Constructor. */
	Entity() {};


	// Protected Attributes
	VectorMap<Component*> m_components;
	MappedChar<function<void(const ECS_Command&)>> m_commandMap;
	friend class EntityCreator;
};

/**
 * An interface to direct the creation of specific entities.
 **/
class DT_ENGINE_API EntityCreator
{
public:
	// (de)Constructors
	/** Virtual Destructor. */
	virtual ~EntityCreator(void) {};
	/** Constructor. 
	 * @param	componentFactory	pointer to the component factory to allow creation of specific components. */
	EntityCreator(Component_Factory * componentFactory) : m_componentFactory(componentFactory) {}


	/** Destroy the entity.
	 * @param	entity	the entity to delete. */
	virtual void destroy(Entity * entity);
	/** Creates an entity
	 * @return	the entity created. */
	virtual Entity* create() = 0;


protected:
	/** Generates a new component.
	 * @param	type	const char array name of the component type to make. */
	Component * makeComponent(const char * type);
	/** Removes a component.
	 * @param	component	the component to destroy */
	void unMakeComponent(Component * component);


private:
	Component_Factory *m_componentFactory;
};

#endif // ENTITY