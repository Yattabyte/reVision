/*
	World_Manager

	- A primitive static namespace container holding a list of level entities
	- Becomes the OWNER of ALL entities it adds
	- It will delete entities when they are removed from the map!
*/

#pragma once
#ifndef WORLD_MANAGER
#define WORLD_MANAGER
#ifdef	DT_CORE_EXPORT
#define DELTA_CORE_API __declspec(dllexport)
#else
#define	DELTA_CORE_API __declspec(dllimport)
#endif

#include "Entities\Entity.h"

using namespace std;

namespace World_Manager {
	// Shutsdown the world and deletes everything 
	DELTA_CORE_API void shutdown();
	// Add the entity to the level
	DELTA_CORE_API void addEntity(Entity * entity);
	// Remove the entity from the level
	DELTA_CORE_API void removeEntity(Entity * entity);
	// Load the world
	DELTA_CORE_API void LoadWorld();
	// Unload the map (remove all entities)
	DELTA_CORE_API void UnloadWorld();
}

#endif // WORLD_MANAGER