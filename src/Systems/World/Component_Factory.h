/*
	Component_Factory

	- Handles creation and storage for all entity components
*/

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
class DT_ENGINE_API Component_Factory {
public:
	~Component_Factory();
	Component_Factory();
	void Initialize(EnginePackage *enginePackage, ECSmessanger *ecsMessange);
	// Creates a component of the supplied type, assigns it the entity parent ID supplied, and returns the handle to this component
	ECShandle CreateComponent(char *type, const ECShandle &parent_ID);
	// Delete the component that matches the supplied ID
	void DeleteComponent(const ECShandle& id);
	// Retrieve the actual component that matches the supplied ID
	Component * GetComponent(const ECShandle& id);
	// Retrieve an array of components that match the category specified
	vector<Component*> &GetComponentsByType(char *type);
	// Removes all components from the system
	void Flush();
	// Returns the data lock for the system
	shared_mutex & GetDataLock();

private:
	bool m_Initialized;
	map<char*, vector<Component*>, cmp_str> m_levelComponents;
	map<char*, deque<unsigned int>> m_freeSpots;
	map<char*, ComponentCreator*, cmp_str> m_creatorMap;
	shared_mutex m_dataLock;
	ECSmessanger *m_ECSmessenger;
	EnginePackage *m_enginePackage;
};

#endif // COMPONENT_FACTORY