/*
	Component_Factory

	- Handles creation and storage for all entity components
*/

#pragma once
#ifndef COMPONENTFACTORY
#define COMPONENTFACTORY
#ifdef	ENGINE_EXPORT
#define DELTA_CORE_API __declspec(dllexport)
#else
#define	DELTA_CORE_API __declspec(dllimport)
#endif

#include "Entities\Components\Component.h"
#include "Systems\ECS\ECSmessage.h"
#include "Rendering\Visibility_Token.h"
#include <map>
#include <vector>
#include <shared_mutex>

using namespace std;

namespace ComponentFactory {
	// Starts up the factory
	DELTA_CORE_API void Startup();
	// Creates a component of the supplied type, assigns it the entity parent ID supplied, and returns the handle to this component
	DELTA_CORE_API ECShandle CreateComponent(char *type, const ECShandle &parent_ID);
	// Delete the component that matches the supplied ID
	DELTA_CORE_API void DeleteComponent(const ECShandle& id);
	// Retrieve the actual component that matches the supplied ID
	DELTA_CORE_API Component * GetComponent(const ECShandle& id);
	// Retrieve an array of components that match the category specified
	DELTA_CORE_API vector<Component*> &GetComponentsByType(char *type);
	// Removes all components from the system
	DELTA_CORE_API void Flush();
	// Returns the data lock for the system
	DELTA_CORE_API shared_mutex & GetDataLock();
}

#endif // COMPONENTFACTORY