/*
	World_Manager

	- A primitive static namespace container holding a list of level entities
	- Becomes the OWNER of ALL entities it adds
	- It will delete entities when they are removed from the map!
*/

#pragma once
#ifndef WORLD_MANAGER
#define WORLD_MANAGER
#ifdef	ENGINE_EXPORT
#define DT_ENGINE_API __declspec(dllexport)
#else
#define	DT_ENGINE_API __declspec(dllimport)
#endif

#include "Entities\Entity.h"

using namespace std;

namespace World_Manager {
	// TO DO
	DT_ENGINE_API void startup();
	// Shutsdown the world and deletes everything 
	DT_ENGINE_API void shutdown();
	// Load the world
	DT_ENGINE_API void LoadWorld();
	// Unload the map (remove all entities)
	DT_ENGINE_API void UnloadWorld();
}

#endif // WORLD_MANAGER