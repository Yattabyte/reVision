/*
	Light

	- An abstract class to be expanded on by all lighting entities
	- To be used in the Lighting Manager
*/

#pragma once
#ifndef LIGHTS
#define LIGHTS
#ifdef	ENGINE_EXPORT
#define DT_ENGINE_API __declspec(dllexport)
#else
#define	DT_ENGINE_API __declspec(dllimport)
#endif

#include "Entities\Entity.h"

class DT_ENGINE_API SpotLightCreator : public EntityCreator
{
public:
	virtual Entity* Create(const ECShandle &id, ECSmessanger *ecsMessanger, Component_Factory *componentFactory) {
		Entity *entity = EntityCreator::Create(id, ecsMessanger, componentFactory);
		entity->addComponent("Light_Spot");
		return entity;
	}
};

#endif // LIGHTS