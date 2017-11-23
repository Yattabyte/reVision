/*
	Sun

	- A type of entity
	- Contains a directional-light-component
*/

#pragma once
#ifndef SUN
#define SUN
#ifdef	DT_CORE_EXPORT
#define DELTA_CORE_API __declspec(dllexport)
#else
#define	DELTA_CORE_API __declspec(dllimport)
#endif

#include "Entities\Entity.h"

class SunCreator : public EntityCreator
{
public:
	DELTA_CORE_API virtual Entity* Create(const ECSHandle &id) {
		Entity *entity = EntityCreator::Create(id);
		entity->addComponent("Light_Directional");
		return entity;
	}
};

#endif // SUN