/*
	Entity

	- The gist of what an entity is
	- Not much to it, as entities need to be specific
*/

#pragma once
#ifndef ENTITY
#define ENTITY
#ifdef	DT_CORE_EXPORT
#define DELTA_CORE_API __declspec(dllexport)
#else
#define	DELTA_CORE_API __declspec(dllimport)
#endif
#define GLEW_STATIC

#include "Systems\ECS\ComponentFactory.h"
#include "GL\glew.h"
#include <map>
#include <vector>

typedef std::pair<char*, unsigned int> ECSHandle;

class ECSMessage;
class EntityCreator;
class Component;
class Entity
{
public:
	DELTA_CORE_API void addComponent(char *type);
	DELTA_CORE_API Component* getComponent(const ECSHandle &id);
	DELTA_CORE_API void IOMessage(ECSMessage *message);
	
protected:
	DELTA_CORE_API virtual ~Entity();
	DELTA_CORE_API Entity(const ECSHandle &id) : m_ID(id) {};
	friend class EntityCreator;
	ECSHandle m_ID;
	std::map<char *, std::vector<unsigned int>, cmp_str> m_component_handles;
};

class EntityCreator
{
public:
	DELTA_CORE_API virtual Entity* Create(const ECSHandle &id) { return new Entity(id); };
	DELTA_CORE_API virtual void Destroy(Entity *entity) { delete entity; };
	DELTA_CORE_API virtual ~EntityCreator(void) {};
};

#endif // ENTITY