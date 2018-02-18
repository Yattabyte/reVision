#pragma once
#ifndef LIGHTS
#define LIGHTS
#ifdef	ENGINE_EXPORT
#define DT_ENGINE_API __declspec(dllexport)
#else
#define	DT_ENGINE_API __declspec(dllimport)
#endif

#include "Entities\Entity.h"


/**
 * Creates a Spot Light entity, composed of only a spot light component.
 **/
class DT_ENGINE_API SpotLightCreator : public EntityCreator
{
public:
	virtual Entity* create(const ECShandle & id, ECSmessenger * ecsMessenger, Component_Factory * componentFactory) {
		Entity *entity = EntityCreator::create(id, ecsMessenger, componentFactory);
		entity->addComponent("Light_Spot");
		return entity;
	}
};

/**
 * Creates a Point Light entity, composed of only a point light component.
 **/
class DT_ENGINE_API PointLightCreator : public EntityCreator
{
public:
	virtual Entity* create(const ECShandle & id, ECSmessenger * ecsMessenger, Component_Factory * componentFactory) {
		Entity *entity = EntityCreator::create(id, ecsMessenger, componentFactory);
		entity->addComponent("Light_Point");
		return entity;
	}
};

#endif // LIGHTS