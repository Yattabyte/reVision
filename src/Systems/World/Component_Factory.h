#pragma once
#ifndef COMPONENT_FACTORY
#define COMPONENT_FACTORY
#ifdef	ENGINE_EXPORT
#define DT_ENGINE_API __declspec(dllexport)
#else
#define	DT_ENGINE_API __declspec(dllimport)
#endif

#include "Entities\Components\Component.h"
#include "Systems\World\ECSmessage.h"
#include "Systems\World\Visibility_Token.h"
#include <deque>
#include <map>
#include <vector>
#include <shared_mutex>

using namespace std;
class ECSmessanger;
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
	/** Initialize  the component factory
	 * @param	enginePackage	pointer to the engine package
	 * @param	ecsMessanger	pointer to the entity-component messenger system */
	void Initialize(EnginePackage * enginePackage, ECSmessanger * ecsMessange);
	/** Creates a component of the supplied type and returns its handle 
	 * @param	type			the type of component to create
	 * @param	parent_ID		the handle of the parent to pass to the component
	 * @return					the handle of the created component */	 
	ECShandle CreateComponent(char * type, const ECShandle & parent_ID);
	/** Delete the component of the given handle.
  	 * @param	id				the handle of the component to delete */
	void DeleteComponent(const ECShandle & id);
	/** Retrieve the actual component that matches the supplied ID.
	 * @param	id				the handle of the component to retrieve
	 * @return					the component who matches the handle provided */
	Component * GetComponent(const ECShandle & id);
	/** Retrieves an array of components that match the category specified.
	 * @param	type			the type-name of the component list to retrieve
	 * @return					the list of components that match the type provided */
	vector<Component*> &GetComponentsByType(char * type);
	/** Removes all components from the system. */
	void Flush();
	/** Returns the data lock for the system. 
	 * @return					the mutex for this factory*/
	shared_mutex & GetDataLock();


private:
	// Private Attributes
	bool m_Initialized;
	map<char*, vector<Component*>, cmp_str> m_levelComponents;
	map<char*, deque<unsigned int>> m_freeSpots;
	map<char*, ComponentCreator*, cmp_str> m_creatorMap;
	shared_mutex m_dataLock;
	ECSmessanger *m_ECSmessenger;
	EnginePackage *m_enginePackage;
};

#endif // COMPONENT_FACTORY