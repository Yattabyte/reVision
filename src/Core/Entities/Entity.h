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

#include "GL\glew.h"
#include <map>
#include <vector>

struct cmp_str { bool operator()(const char *a, const char *b) const { return std::strcmp(a, b) < 0; } };

class EntityCreator;
class Component;
class Entity
{
public:
	DELTA_CORE_API Component* addComponent(char *type);
	
protected:
	DELTA_CORE_API virtual ~Entity();
	DELTA_CORE_API Entity() {};
	friend class EntityCreator;
	std::map<char *, std::vector<unsigned int>, cmp_str> m_component_handles;
};

class EntityCreator
{
public:
	DELTA_CORE_API virtual Entity* Create(void) { return new Entity(); };
	DELTA_CORE_API virtual void Destroy(Entity *entity) { delete entity; };
	DELTA_CORE_API virtual ~EntityCreator(void) {};
};

#endif // ENTITY