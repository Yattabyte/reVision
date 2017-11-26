/*
	Component

	- An extendable container like object
	- To be derived/inherited from to accomplish a specific goal
*/

#pragma once
#ifndef COMPONENT
#define COMPONENT
#ifdef	ENGINE_EXPORT
#define DT_ENGINE_API __declspec(dllexport)
#else
#define	DT_ENGINE_API __declspec(dllimport)
#endif

#include "Systems\ECS\ECSmessage.h"
#include <utility>

class ComponentCreator;
class DT_ENGINE_API Component
{
public:
	// Propogates a message from this component to its parent
	void SendMessage(const ECSmessage &message);
	// Handles what to do when receiving a message
	virtual void ReceiveMessage(const ECSmessage &message);
	// Returns whether or not the provided message was sent from this component
	bool Am_I_The_Sender(const ECSmessage &message);

protected:
	virtual ~Component() {};
	Component(const ECShandle &id, const ECShandle &pid) : m_ID(id), m_parentID(pid) {};
	ECShandle m_ID, m_parentID;
	friend class ComponentCreator;
};

class DT_ENGINE_API ComponentCreator
{
public:
	virtual Component* Create(const ECShandle &id, const ECShandle &pid) { return new Component(id, pid); };
	void Destroy(Component *component) { delete component; };
	virtual ~ComponentCreator(void) {};
};

#endif // COMPONENT