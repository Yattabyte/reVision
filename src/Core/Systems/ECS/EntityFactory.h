/*
	Entity_Factory

	- Handles creation and storage for all level entities
*/

#pragma once
#ifndef ENTITYFACTORY
#define ENTITYFACTORY
#ifdef	ENGINE_EXPORT
#define DT_ENGINE_API __declspec(dllexport)
#else
#define	DT_ENGINE_API __declspec(dllimport)
#endif

#include "Entities\Entity.h"
#include "Systems\ECS\ECSmessage.h"
#include <vector>

using namespace std;

namespace EntityFactory {
	// Starts up the factory
	DT_ENGINE_API void Startup();
	// Creates an entity of the supplied type and returns the handle
	DT_ENGINE_API ECShandle CreateEntity(char *type);
	// Delete the entity that matches the supplied ID
	DT_ENGINE_API void DeleteEntity(const ECShandle& id);
	// Retrieve the actual entity that matches the supplied ID
	DT_ENGINE_API Entity * GetEntity(const ECShandle& id);
	// Retrieve an array of entities that match the category specified
	DT_ENGINE_API vector<Entity*> &GetEntitiesByType(char *type);
	// Removes all entities from the system
	DT_ENGINE_API void Flush();
}

#endif // ENTITYFACTORY