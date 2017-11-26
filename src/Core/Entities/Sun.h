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

class SunCreator : public EntityCreator
{
public:
	DT_ENGINE_API virtual Entity* Create(const ECShandle &id) {
		Entity *entity = EntityCreator::Create(id);
		entity->addComponent("Light_Directional");
		return entity;
	}
};

#endif // SUN