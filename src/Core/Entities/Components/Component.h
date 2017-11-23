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

#include "Systems\ECS\ECSmessage.h"
#include <utility>

class ComponentCreator;
class Component
{
public:
	DELTA_CORE_API void SendMessage(ECSmessage &message);
	DELTA_CORE_API virtual void ReceiveMessage(ECSmessage &message);

protected:
	DELTA_CORE_API virtual ~Component() {};
	DELTA_CORE_API Component(const ECSHandle &id, const ECSHandle &pid) : m_ID(id), m_parentID(pid) {};
	ECSHandle m_ID, m_parentID;
	friend class ComponentCreator;
};

class DELTA_CORE_API ComponentCreator
{
public:
	virtual Component* Create(const ECSHandle &id, const ECSHandle &pid) { return new Component(id, pid); };
	void Destroy(Component *component) { delete component; };
	virtual ~ComponentCreator(void) {};
};

#endif // COMPONENT