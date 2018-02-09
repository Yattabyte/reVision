/*
	Sun

	- A type of entity
	- Contains a directional-light-component
*/

#pragma once
#ifndef SUN
#define SUN
#ifdef	ENGINE_EXPORT
#define DT_ENGINE_API __declspec(dllexport)
#else
#define	DT_ENGINE_API __declspec(dllimport)
#endif

#include "Entities\Entity.h"

class DT_ENGINE_API SunCreator : public EntityCreator
{
public:
	virtual Entity* Create(const ECShandle &id, ECSmessanger *ecsMessanger, Component_Factory *componentFactory) {
		Entity *entity = EntityCreator::Create(id, ecsMessanger, componentFactory);
		entity->addComponent("Light_Directional");
		return entity;
	}
};

#endif // SUN