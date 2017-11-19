/*
	Prop

	- A type of entity
	- Contains an animation-supported model-component
*/

#pragma once
#ifndef PROP
#define PROP
#ifdef	DT_CORE_EXPORT
#define DELTA_CORE_API __declspec(dllexport)
#else
#define	DELTA_CORE_API __declspec(dllimport)
#endif

#include "Entities\Entity.h"

class PropCreator : protected EntityCreator
{
public:
	DELTA_CORE_API virtual Entity* Create(const ECSHandle &id) {
		Entity *entity = EntityCreator::Create(id);
		entity->addComponent("Anim_Model");
		return entity;
	}
};

#endif // PROP