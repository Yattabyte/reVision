/*
	Entity_Factory

	- Handles creation and storage for all level entities
*/

#pragma once
#ifndef ENTITYFACTORY
#define ENTITYFACTORY
#ifdef	DT_CORE_EXPORT
#define DELTA_CORE_API __declspec(dllexport)
#else
#define	DELTA_CORE_API __declspec(dllimport)
#endif

#include "Entities\Entity.h"
#include "Systems\ECS\ECSmessage.h"
#include <vector>

using namespace std;

namespace EntityFactory {
	// Starts up the factory
	DELTA_CORE_API void Startup();
	// Creates an entity of the supplied type and returns the handle
	DELTA_CORE_API ECSHandle CreateEntity(char *type);
	// Delete the entity that matches the supplied ID
	DELTA_CORE_API void DeleteEntity(const ECSHandle& id);
	// Retrieve the actual entity that matches the supplied ID
	DELTA_CORE_API Entity * GetEntity(const ECSHandle& id);
	// Retrieve an array of entities that match the category specified
	DELTA_CORE_API vector<Entity*> &GetEntitiesByType(char *type);
	// Sends a message to the entity with the handle supplied
	DELTA_CORE_API void SendMessageToEntity(ECSmessage &message, const ECSHandle &target);
	// Removes all entities from the system
	DELTA_CORE_API void Flush();
}

#endif // ENTITYFACTORY