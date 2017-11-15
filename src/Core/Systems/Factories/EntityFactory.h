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
#include <vector>

using namespace std;

namespace EntityFactory {
	DELTA_CORE_API void Startup();
	DELTA_CORE_API unsigned int CreateEntity(char *type);
	DELTA_CORE_API void DeleteEntity(char *type, const unsigned int &id);
	DELTA_CORE_API Entity * GetEntity(char *type, const unsigned int &id);
	DELTA_CORE_API vector<Entity*> &GetEntitiesByType(char *type);
	DELTA_CORE_API void Flush();
}

#endif // ENTITYFACTORY