/*
	Entity

	- The gist of what an entity is
	- Not much to it, as entities need to be specific
*/

#pragma once
#ifndef ENTITY
#define ENTITY
#ifdef	ENGINE_EXPORT
#define DT_ENGINE_API __declspec(dllexport)
#else
#define	DT_ENGINE_API __declspec(dllimport)
#endif
#define GLEW_STATIC

#include "Systems\World\ECSmessage.h"
#include "Systems\World\ECSdefines.h"
#include "GL\glew.h"
#include <map>
#include <vector>


class ECSmessanger;
class Component_Factory;
class EntityCreator;
class Component;
class DT_ENGINE_API Entity
{
public:
	void addComponent(char *type);
	Component* getComponent(const ECShandle &id);
	void SendMessage(const ECSmessage &message);
	void ReceiveMessage(const ECSmessage &message);
	
protected:
	virtual ~Entity();
	Entity(const ECShandle &id) : m_ID(id) {};
	ECShandle m_ID;
	std::map<char *, std::vector<unsigned int>, cmp_str> m_component_handles;
	ECSmessanger *m_ECSmessanger;
	Component_Factory *m_componentFactory;
	friend class EntityCreator;
};

class DT_ENGINE_API EntityCreator
{
public:
	virtual ~EntityCreator(void) {};
	virtual void Destroy(Entity *entity) { delete entity; };
	virtual Entity* Create(const ECShandle &id, ECSmessanger *ecsMessanger, Component_Factory *componentFactory) { 
		Entity *entity = new Entity(id);
		entity->m_ECSmessanger = ecsMessanger;
		entity->m_componentFactory = componentFactory;
		return entity;
	};
};

#endif // ENTITY