#pragma once
#ifndef WORLD_MODULE_H
#define WORLD_MODULE_H

#include "Modules/Engine_Module.h"
#include "Modules/ECS/ecsComponent.h"
#include "Modules/ECS/ecsEntity.h"
#include "Modules/ECS/ecsSystem.h"
#include "Utilities/MappedChar.h"
#include <map>
#include <functional>
#include <vector>


/** A module responsible for the world/level. */
class World_Module final : public Engine_Module {
public:
	// Public Enumerations
	const enum WorldState {
		unloaded,
		startLoading,
		finishLoading,
		updated
	};


	// Public (de)Constructors
	/** Destroy this world module. */
	inline ~World_Module() = default;
	/** Construct a world module. */
	inline World_Module() = default;


	// Public Interface Implementations
	virtual void initialize(Engine* engine) override final;
	virtual void deinitialize() override final;
	virtual void frameTick(const float& deltaTime) override final;


	// Public Methods
	/** Loads the world, specified by the map name.
	@param	mapName				the name of the map to load. */
	void loadWorld(const std::string& mapName);
	/** Saves the world with a specified map name.
	@param	mapName				the name of the map to save as. */
	void saveWorld(const std::string& mapName);
	/** Unload the current world. */
	void unloadWorld();	
	/** Registers a notification function to be called when the world state changes.
	@param	alive				a shared pointer indicating whether the caller is still alive or not.
	@param	notifier			function to be called on state change. */
	void addLevelListener(const std::shared_ptr<bool>& alive, const std::function<void(const WorldState&)>& func);


private:
	// Private Methods
	/** Notify all world-listeners of a state change.
	@param	state				the new state to notify listeners of. */
	void notifyListeners(const WorldState& state);
	

	// Private Attributes
	bool m_finishedLoading = false;
	WorldState m_state = unloaded;
	std::vector<std::pair<std::shared_ptr<bool>, std::function<void(const WorldState&)>>> m_notifyees;
	std::shared_ptr<bool> m_aliveIndicator = std::make_shared<bool>(true);
};

#endif // WORLD_MODULE_h