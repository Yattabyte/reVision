#pragma once
#ifndef PROP
#define PROP
#ifdef	ENGINE_EXPORT
#define DT_ENGINE_API __declspec(dllexport)
#else
#define	DT_ENGINE_API __declspec(dllimport)
#endif

#include "Entities\Entity.h"


/**
 * Creates a Prop entity, composed of an animated model component
 **/
class DT_ENGINE_API PropCreator : public EntityCreator
{
public:
	virtual Entity* create(const ECShandle & id, ECSmessenger * ecsMessenger, Component_Factory * componentFactory) {
		Entity *entity = EntityCreator::create(id, ecsMessenger, componentFactory);
		entity->addComponent("Anim_Model");
		return entity;
	}
};

#endif // PROP