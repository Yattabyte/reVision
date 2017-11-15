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
#include <vector>

class Component;
class Entity
{
public:
	Entity() {};
	DELTA_CORE_API ~Entity();
	DELTA_CORE_API void addComponent(Component *newComponent);
	
protected:
	std::vector<std::pair<unsigned int, unsigned int>> m_component_handles;
};

#endif // ENTITY