/*
	ECSmessanger

	- Handles delivery of messages between entities and components
*/

#pragma once
#ifndef ECSMESSANGER
#define ECSMESSANGER
#ifdef	ENGINE_EXPORT
#define DT_ENGINE_API __declspec(dllexport)
#else
#define	DT_ENGINE_API __declspec(dllimport)
#endif

#include "Systems\World\ECSmessage.h"
#include "Systems\World\ECSdefines.h"
#include <map>
#include <vector>

class Entity_Factory;
class Component_Factory;

class DT_ENGINE_API ECSmessanger {
public:
	~ECSmessanger();
	ECSmessanger(Entity_Factory *entityFactory, Component_Factory *componentFactory);

	void SendMessage_ToEntity(const ECSmessage &message, const ECShandle &target);
	void SendMessage_ToComponent(const ECSmessage &message, const ECShandle &target);
	void SendMessage_ToComponents(const ECSmessage &message, const ECShandle_map &targets);

private:
	Entity_Factory *m_entityFactory;
	Component_Factory *m_componentFactory;
};

#endif // ECSMESSANGER
