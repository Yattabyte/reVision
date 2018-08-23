#pragma once
#ifndef WORLD_MODULE_H
#define WORLD_MODULE_H

#include "Modules\Engine_Module.h"
#include <vector>


/** A module responsible for the world/level. */
class World_Module : public Engine_Module {
public:
	// (de)Constructors
	~World_Module();
	World_Module(Engine * engine);


	// Public Methods
	/** Loads the world. */
	void loadWorld();
	/** Registers a notification flag to be updated when level loaded.
	@param	notifier	flag to be set when level loaded*/
	void addLevelListener(bool * notifier);
	/** Checks whether the level has finished loading. */
	void checkIfLoaded();


private:
	// Private Attributes
	bool m_finishedLoading;
	std::vector<bool*> m_notifyees;
};

#endif // WORLD_MODULE_h