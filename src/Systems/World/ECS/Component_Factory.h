#pragma once
#ifndef COMPONENT_FACTORY_H
#define COMPONENT_FACTORY_H

#include "Systems\World\ECS\Components\Component.h"
#include "Systems\World\ECS\ECSmessage.h"
#include "Systems\World\Visibility_Token.h"
#include "Utilities\MappedChar.h"
#include <deque>
#include <shared_mutex>

using namespace std;
class EnginePackage;


/**
 * A utility that handles the creation and storage of all level components
 **/
class Component_Factory {
public:
	// (de)Constructors
	/** Destroy the factory. */
	~Component_Factory();
	/** Construct the factory. */
	Component_Factory();

	
	// Public Methods
	/** Initialize  the component factory.
	 * @param	enginePackage	pointer to the engine package */
	void initialize(EnginePackage * enginePackage);
	/** Creates a component of the supplied type and returns its handle.
	 * @param	type			the type of component to create
	 * @return					the newely created component */	 
	Component * createComponent(const char * type);
	/** Deletes the component provided.
  	 * @param	component		the component to delete */
	void deleteComponent(Component * component);	
	/** Retrieves an array of components that match the category specified.
	 * @brief					Guaranteed to return at least a zero-length vector. Types that don't exist are created.
	 * @param	type			the type-name of the component list to retrieve
	 * @return					the list of components that match the type provided */
	const vector<Component*> & getComponentsByType(const char * type);
	/** Removes all components from the system. */
	void flush();	
	/** Checks the map to see if it has any entries of a specific type.
	 * @param	key				the type to check
	 * @return					true if it finds the key in the map, false otherwise */
	bool find(const char * key) const;
	/** Returns the data lock for the system. 
	 * @return					the mutex for this factory */
	shared_mutex & getDataLock();


private:
	// Private Attributes
	bool m_Initialized;
	VectorMap<Component*> m_levelComponents;	
	MappedChar<deque<unsigned int>> m_freeSpots;
	MappedChar<ComponentCreator*> m_creatorMap;
	mutable shared_mutex m_dataLock;
	EnginePackage *m_enginePackage;
};

#endif // COMPONENT_FACTORY_H