/*
	World

	- A system that encompases the world state of the engine
*/



#pragma once
#ifndef SYSTEM_WORLD
#define SYSTEM_WORLD
#ifdef	ENGINE_EXPORT
#define DT_ENGINE_API __declspec(dllexport)
#else
#define	DT_ENGINE_API __declspec(dllimport)
#endif

#include "Systems\System_Interface.h"
#include "Systems\World\ECSmessanger.h"
#include "Systems\World\Entity_Factory.h"
#include "Systems\World\Component_Factory.h"

class Engine_Package;
class DT_ENGINE_API System_World : public System
{
public: 
	~System_World();
	System_World();
	void Initialize(Engine_Package *enginePackage);

	// Recalculate visibility
	void Update(const float &deltaTime);
	void Update_Threaded(const float &deltaTime);


private:
	Entity_Factory m_entityFactory;
	Component_Factory m_componentFactory;
	ECSmessanger m_ECSmessanger;
};

#endif // SYSTEM_WORLD