#pragma once
#ifndef SYSTEM_WORLD_H
#define SYSTEM_WORLD_H

#include "Systems\System_Interface.h"
#include "Systems\World\Camera.h"


class Engine;

/**
 * An engine system representing a world / level state.
 **/
class System_World : public System
{
public:
	// (de)Constructors
	/** Destroy the world system. */
	~System_World();
	/** Construct the world system. */
	System_World();


	// Interface Implementations
	virtual void initialize(Engine * engine);
	virtual void update(const float & deltaTime);
	virtual void updateThreaded(const float & deltaTime);


	// Public Methods
	/** Register a viewer into the system, to maintain its visibility info. 
 	 * @param	camera	the camera to register */
	void registerViewer(Camera * camera);
	/** Remove a viewer from the system.
     * @param	camera	the camera to unregister */
	void unregisterViewer(Camera * camera);
	/** Submit a bool flag to update to true when the level finishes loading.
	 * @param	notifyee	the bool flag to update */
	void notifyWhenLoaded(bool * notifyee);


private:
	// Private Methods
	void calcVisibility(Camera & camera);
	void loadWorld();
	void unloadWorld();
	void checkWorld();
	void saveWorld();


	// Private Attributes
	std::shared_mutex m_viewerLock;
	std::vector<Camera*> m_viewers;
	std::vector<bool *> m_loadNotifiers;
	std::shared_mutex m_stateLock;
	bool m_loaded, m_worldChanged;
};

#endif // SYSTEM_WORLD_H