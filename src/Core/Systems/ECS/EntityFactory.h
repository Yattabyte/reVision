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

class ECSmessage;
namespace EntityFactory {
	DELTA_CORE_API void Startup();
	DELTA_CORE_API ECSHandle CreateEntity(char *type);
	DELTA_CORE_API void DeleteEntity(const ECSHandle& id);
	DELTA_CORE_API Entity * GetEntity(const ECSHandle& id);
	DELTA_CORE_API vector<Entity*> &GetEntitiesByType(char *type);
	DELTA_CORE_API void SendMessageToEntity(ECSmessage *message, const ECSHandle &target);
	DELTA_CORE_API void Flush();
}

#endif // ENTITYFACTORY