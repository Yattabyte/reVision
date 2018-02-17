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


/**
 * An engine system representing a world / level state.
 **/
class DT_ENGINE_API System_World : public System
{
public:
	// (de)Constructors
	/** Destroy the world system. */
	~System_World();
	/** Construct the world system. */
	System_World();


	// Interface Implementations
	virtual void initialize(EnginePackage *enginePackage);
	virtual void update(const float &deltaTime);
	virtual void updateThreaded(const float &deltaTime);


	// Public Methods
	/** Register a viewer into the system, to maintain its visibility info. 
 	 * @param	camera	the camera to register */
	void registerViewer(Camera *camera);
	/** Remove a viewer from the system.
	* @param	camera	the camera to unregister */
	void unregisterViewer(Camera *camera);
	/** Retrieve and down-cast an array of components that match the category specified.
	 * @param	type	the name of the component type to retrieve
	 * @param	<T>		the class-type to cast the components to */
	template <typename T>
	vector<T*> &getSpecificComponents(char *type) {
		auto &found = m_componentFactory.GetComponentsByType(type);
		if (found.size())
			return *(vector<T*>*)(&found);
		return vector<T*>();
	}


private:
	// Private Methods
	void calcVisibility(Camera &camera);


	// Private Attributes
	Entity_Factory m_entityFactory;
	Component_Factory m_componentFactory;
	ECSmessanger m_ECSmessenger;
	shared_mutex m_lock;
	vector<Camera*> m_viewers;
};

#endif // SYSTEM_WORLD