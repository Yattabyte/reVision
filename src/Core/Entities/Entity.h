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

#include "Systems\ECS\ComponentFactory.h"
#include "Systems\ECS\ECSmessage.h"
#include "Systems\ECS\ECSdefines.h"
#include "GL\glew.h"
#include <map>
#include <vector>



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
	friend class EntityCreator;
	ECShandle m_ID;
	std::map<char *, std::vector<unsigned int>, cmp_str> m_component_handles;
};

class DT_ENGINE_API EntityCreator
{
public:
	virtual Entity* Create(const ECShandle &id) { return new Entity(id); };
	virtual void Destroy(Entity *entity) { delete entity; };
	virtual ~EntityCreator(void) {};
};

#endif // ENTITY