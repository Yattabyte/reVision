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
#include "Systems\World\Camera.h"
#include "Systems\World\ECSmessanger.h"
#include "Systems\World\Entity_Factory.h"
#include "Systems\World\Component_Factory.h"

class EnginePackage;
class DT_ENGINE_API System_World : public System
{
public: 
	~System_World();
	System_World();
	void Initialize(EnginePackage *enginePackage);

	void Update(const float &deltaTime);
	void Update_Threaded(const float &deltaTime);
	void RegisterViewer(Camera *c);
	void UnRegisterViewer(Camera *c);
	// Retrieve an array of components that match the category specified
	template <typename T>
	vector<T*> &GetSpecificComponents(char *type) {
		auto &found = m_componentFactory.GetComponentsByType(type);
		if (found.size())
			return *(vector<T*>*)(&found);
		return vector<T*>();
	}

private:
	Entity_Factory m_entityFactory;
	Component_Factory m_componentFactory;
	ECSmessanger m_ECSmessenger;
	shared_mutex m_lock;
	vector<Camera*> m_viewers;

	void calcVisibility(Camera &camera);
};

#endif // SYSTEM_WORLD