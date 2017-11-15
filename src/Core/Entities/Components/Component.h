/*
	Component

	- An extendable container like object
	- To be derived/inherited from to accomplish a specific goal
*/

#pragma once
#ifndef COMPONENT
#define COMPONENT
#ifdef	DT_CORE_EXPORT
#define DELTA_CORE_API __declspec(dllexport)
#else
#define	DELTA_CORE_API __declspec(dllimport)
#endif

class ComponentCreator;
class Component
{
public:
	virtual unsigned int GetTypeID() { return -1; }

protected:
	DELTA_CORE_API virtual ~Component() {};
	DELTA_CORE_API Component() {};
	friend class ComponentCreator;
};

class ComponentCreator
{
public:
	DELTA_CORE_API virtual Component* Create(void) { return new Component(); };
	DELTA_CORE_API virtual void Destroy(Component *component) { delete component; };
	DELTA_CORE_API virtual ~ComponentCreator(void) {};
};

#endif // COMPONENT