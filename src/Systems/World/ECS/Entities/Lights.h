#pragma once
#ifndef LIGHTS
#define LIGHTS
#ifdef	ENGINE_EXE_EXPORT
#define DT_ENGINE_API 
#elif	ENGINE_DLL_EXPORT 
#define DT_ENGINE_API __declspec(dllexport)
#else
#define	DT_ENGINE_API __declspec(dllimport)
#endif

#include "Systems\World\ECS\Entities\Entity.h"


/**
 * Creates a Spot Light entity, composed of only a spot light component.
 **/
class DT_ENGINE_API Creator_SpotLight : public EntityCreator
{
public:
	virtual Entity* create(const ECShandle & id, ECSmessenger * ecsMessenger, Component_Factory * componentFactory) {
		Entity *entity = EntityCreator::create(id, ecsMessenger, componentFactory);
		entity->addComponent("Light_Spot");
		return entity;
	}
};

/**
 * Creates a Cheap Spot Light entity, composed of only a cheap spot light component.
 **/
class DT_ENGINE_API Creator_SpotLight_Cheap : public EntityCreator
{
public:
	virtual Entity* create(const ECShandle & id, ECSmessenger * ecsMessenger, Component_Factory * componentFactory) {
		Entity *entity = EntityCreator::create(id, ecsMessenger, componentFactory);
		entity->addComponent("Light_Spot_Cheap");
		return entity;
	}
};

/**
 * Creates a Point Light entity, composed of only a point light component.
 **/
class DT_ENGINE_API Creator_PointLight : public EntityCreator
{
public:
	virtual Entity* create(const ECShandle & id, ECSmessenger * ecsMessenger, Component_Factory * componentFactory) {
		Entity *entity = EntityCreator::create(id, ecsMessenger, componentFactory);
		entity->addComponent("Light_Point");
		return entity;
	}
};

/**
 * Creates a Cheap Point Light entity, composed of only a cheap point light component.
 **/
class DT_ENGINE_API Creator_PointLight_Cheap : public EntityCreator
{
public:
	virtual Entity* create(const ECShandle & id, ECSmessenger * ecsMessenger, Component_Factory * componentFactory) {
		Entity *entity = EntityCreator::create(id, ecsMessenger, componentFactory);
		entity->addComponent("Light_Point_Cheap");
		return entity;
	}
};

#endif // LIGHTS