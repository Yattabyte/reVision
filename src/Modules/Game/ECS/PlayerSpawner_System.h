#pragma once
#pragma once
#ifndef PLAYERSPAWNER_SYSTEM_H
#define PLAYERSPAWNER_SYSTEM_H

#include "Modules/ECS/ecsSystem.h"


// Forward Declarations
class Engine;
class Game_Module;

/** A system responsible for spawning players at a specific spawn point. */
class PlayerSpawn_System final : public ecsBaseSystem {
public:
	// Public (De)Constructors
	/** Destroy this free-look system. */
	inline ~PlayerSpawn_System() noexcept {
		// Update indicator
		*m_aliveIndicator = false;
	}
	/** Construct a free-look system.
	@param	engine		reference to the engine to use. 
	@param	game		reference to the game module to use. */
	PlayerSpawn_System(Engine& engine, Game_Module& game);


	// Public Interface Implementation
	void updateComponents(const float& deltaTime, const std::vector<std::vector<ecsBaseComponent*>>& components) final;


private:
	// Private Attributes
	Engine& m_engine;
	Game_Module& m_game;
	size_t m_playerCount = 0ull;
	EntityHandle m_playerHandle;
	std::shared_ptr<bool> m_aliveIndicator = std::make_shared<bool>(true);
};

#endif // PLAYERSPAWNER_SYSTEM_H