#pragma once
#ifndef COMPONENT_FACTORY
#define COMPONENT_FACTORY
#ifdef	ENGINE_EXE_EXPORT
#define DT_ENGINE_API 
#elif	ENGINE_DLL_EXPORT 
#define DT_ENGINE_API __declspec(dllexport)
#else
#define	DT_ENGINE_API __declspec(dllimport)
#endif

#include "Systems\World\ECS\Components\Component.h"
#include "Systems\World\ECS\ECSmessage.h"
#include "Systems\World\Visibility_Token.h"
#include "Utilities\MappedChar.h"
#include <deque>
#include <shared_mutex>

using namespace std;
class ECSmessenger;
class EnginePackage;


/**
 * A utility that handles the creation and storage of all level components
 **/
class DT_ENGINE_API Component_Factory {
public:
	// (de)Constructors
	/** Destroy the factory. */
	~Component_Factory();
	/** Construct the factory. */
	Component_Factory();

	
	// Public Methods
	/** Initialize  the component factory.
	 * @param	enginePackage	pointer to the engine package
	 * @param	ecsMessenger	pointer to the entity-component messenger system */
	void initialize(EnginePackage * enginePackage, ECSmessenger * ecsMessange);
	/** Creates a component of the supplied type and returns its handle.
	 * @param	type			the type of component to create
	 * @param	parent_ID		the handle of the parent to pass to the component
	 * @return					the handle of the created component */	 
	ECShandle createComponent(const char * type, const ECShandle & parent_ID);
	/** Delete the component of the given handle.
  	 * @param	id				the handle of the component to delete */
	void deleteComponent(const ECShandle & id);
	/** Retrieve the actual component that matches the supplied ID.
	 * @param	id				the handle of the component to retrieve
	 * @return					the component who matches the handle provided */
	Component * getComponent(const ECShandle & id);
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
	ECSmessenger *m_ECSmessenger;
	EnginePackage *m_enginePackage;
};

#endif // COMPONENT_FACTORY