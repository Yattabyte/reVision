#pragma once
#ifndef WORLD_MODULE_H
#define WORLD_MODULE_H

#include "Modules/Engine_Module.h"
#include "Assets/Level.h"
#include "Utilities/ECS/ECS.h"
#include "Utilities/MappedChar.h"
#include <vector>


/** A module responsible for the world/level. */
class World_Module : public Engine_Module {
public:
	// (de)Constructors
	~World_Module();
	World_Module() = default;


	// Public Interface Implementation
	/** Initialize the module. */
	virtual void initialize(Engine * engine) override;
	/** Tick the world by a frame.
	@param	deltaTime	the amount of time passed since last frame */
	virtual void frameTick(const float & deltaTime) override;


	// Public Methods
	/** Loads the world. */
	void loadWorld(const std::string & mapName);
	/** Registers a notification flag to be updated when level loaded.
	@param	notifier	flag to be set when level loaded*/
	void addLevelListener(bool * notifier);
	/** Checks whether the level has finished loading. 
	@return				true if level sufficiently loaded, false otherwise. */
	const bool checkIfLoaded();


private:
	// Private Methods
	/** Process the level asset, generating components and entities. */
	void processLevel();


	// Private Attributes
	bool m_finishedLoading = false;
	std::vector<bool*> m_notifyees;
	Shared_Level m_level;
	std::shared_ptr<bool> m_aliveIndicator = std::make_shared<bool>(true);
};

#endif // WORLD_MODULE_h