#pragma once
#ifndef SUN
#define SUN
#ifdef	ENGINE_EXPORT
#define DT_ENGINE_API __declspec(dllexport)
#else
#define	DT_ENGINE_API __declspec(dllimport)
#endif

#include "Entities\Entity.h"


/**
 * Creates a Sun entity, composed of only a directional light component.
 **/
class DT_ENGINE_API SunCreator : public EntityCreator
{
public:
	virtual Entity* create(const ECShandle & id, ECSmessanger * ecsMessanger, Component_Factory * componentFactory) {
		Entity *entity = EntityCreator::create(id, ecsMessanger, componentFactory);
		entity->addComponent("Light_Directional");
		return entity;
	}
};

#endif // SUN