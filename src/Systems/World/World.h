#pragma once
#ifndef SYSTEM_WORLD
#define SYSTEM_WORLD
#ifdef	ENGINE_EXE_EXPORT
#define DT_ENGINE_API 
#elif	ENGINE_DLL_EXPORT 
#define DT_ENGINE_API __declspec(dllexport)
#else
#define	DT_ENGINE_API __declspec(dllimport)
#endif

#include "Systems\System_Interface.h"
#include "Systems\World\Camera.h"
#include "Systems\World\Animator.h"
#include "Systems\World\ECS\Entity_Factory.h"
#include "Systems\World\ECS\Component_Factory.h"

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
	virtual void initialize(EnginePackage * enginePackage);
	virtual void update(const float & deltaTime);
	virtual void updateThreaded(const float & deltaTime);


	// Public Methods
	/** Register a viewer into the system, to maintain its visibility info. 
 	 * @param	camera	the camera to register */
	void registerViewer(Camera * camera);
	/** Remove a viewer from the system.
	* @param	camera	the camera to unregister */
	void unregisterViewer(Camera * camera);
	/** Retrieve and down-cast an array of components that match the category specified.
	 * @brief			Guaranteed to return at least a zero-length vector. Types that don't exist are created.
	 * @param	type	the name of the component type to retrieve
	 * @param	<T>		the class-type to cast the components to */
	template <typename T>
	const vector<T*> getSpecificComponents(const char * type) {
		// Want to return a copy because this data would need to be locked until done being used at its target otherwise.
		shared_lock<shared_mutex> read_lock(m_componentFactory.getDataLock());
		return *(vector<T*>*)(&m_componentFactory.getComponentsByType(type));
	}
	/** Returns true if the world is loaded, as far as its entities are concerned, otherwise false. */
	bool isWorldLoaded() const;


private:
	// Private Methods
	void calcVisibility(Camera & camera);
	void loadWorld();
	void checkWorld();


	// Private Attributes
	Entity_Factory m_entityFactory;
	Component_Factory m_componentFactory;
	shared_mutex m_lock;
	vector<Camera*> m_viewers;
	Animator m_animator;
	bool m_loaded, m_worldChanged;
};

#endif // SYSTEM_WORLD