#pragma once
#ifndef SUN_CHEAP
#define SUN_CHEAP
#ifdef	ENGINE_EXE_EXPORT
#define DT_ENGINE_API 
#elif	ENGINE_DLL_EXPORT 
#define DT_ENGINE_API __declspec(dllexport)
#else
#define	DT_ENGINE_API __declspec(dllimport)
#endif

#include "Systems\World\ECS\Entities\Entity.h"


/**
 * Creates a Cheap Sun entity, composed of only a cheap directional light component.
 **/
class DT_ENGINE_API SunCheapCreator : public EntityCreator
{
public:
	virtual Entity* create(const ECShandle & id, ECSmessenger * ecsMessenger, Component_Factory * componentFactory) {
		Entity *entity = EntityCreator::create(id, ecsMessenger, componentFactory);
		entity->addComponent("Light_Directional_Cheap");
		return entity;
	}
};

#endif // SUN_CHEAP