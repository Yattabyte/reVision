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

class Entity
{
public:
	// Destructs the entity
	~Entity() {};
	// Constructs the entity
	Entity() {};
	// Constructs the entity from another entity
	Entity(const Entity &other) {};
	// Change this entity into another entity
	void operator= (const Entity &other) {};
	// Tell this entity to register itself into any and all subsystems that it requires
	virtual void registerSelf() {};
	// Tell this entity to un-register itself from any and all subsystems that it required
	virtual void unregisterSelf() {};
};

#endif // ENTITY