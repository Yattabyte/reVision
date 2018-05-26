#pragma once
#ifndef PROP
#define PROP
#ifdef	ENGINE_EXE_EXPORT
#define DT_ENGINE_API 
#elif	ENGINE_DLL_EXPORT 
#define DT_ENGINE_API __declspec(dllexport)
#else
#define	DT_ENGINE_API __declspec(dllimport)
#endif

#include "Systems\World\ECS\Entities\Entity.h"


/**
 * Creates a Prop entity, composed of an animated model component
 **/
class DT_ENGINE_API Creator_Prop : public EntityCreator
{
public:
	virtual Entity* create(const ECShandle & id, ECSmessenger * ecsMessenger, Component_Factory * componentFactory) {
		Entity *entity = EntityCreator::create(id, ecsMessenger, componentFactory);
		entity->addComponent("Anim_Model");
		return entity;
	}
};

/**
* Creates a Prop entity, composed of an animated model component
**/
class DT_ENGINE_API Creator_Prop_Static : public EntityCreator
{
public:
	virtual Entity* create(const ECShandle & id, ECSmessenger * ecsMessenger, Component_Factory * componentFactory) {
		Entity *entity = EntityCreator::create(id, ecsMessenger, componentFactory);
		entity->addComponent("Static_Model");
		return entity;
	}
};

#endif // PROP