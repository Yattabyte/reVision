#pragma once
#pragma once
#ifndef PLAYERSPAWNER_SYSTEM_H
#define PLAYERSPAWNER_SYSTEM_H

#include "Modules/ECS/ecsSystem.h"
#include "Modules/ECS/component_types.h"
#include "Modules/Game/Game_M.h"
#include "Engine.h"
#include "glm/glm.hpp"


/** A system responsible for spawning players at a specific spawn point. */
class PlayerSpawn_System final : public ecsBaseSystem {
public:
	// Public (de)Constructors
	/** Destroy this free-look system. */
	inline ~PlayerSpawn_System() {
		// Update indicator
		*m_aliveIndicator = false;
	}
	/** Construct a free-look system. */
	inline PlayerSpawn_System(Engine* engine, Game_Module* game)
		: m_engine(engine), m_game(game) {
		// Declare component types used
		addComponentType(PlayerSpawn_Component::m_ID, FLAG_REQUIRED);
		addComponentType(Transform_Component::m_ID, FLAG_REQUIRED);

		// Error Reporting
		if (!isValid())
			engine->getManager_Messages().error("Invalid ECS System: PlayerSpawn_System");
	}


	// Public Interface Implementation
	inline virtual void updateComponents(const float& deltaTime, const std::vector< std::vector<ecsBaseComponent*> >& components) override final {
		auto& graphicsModule = m_engine->getModule_Graphics();
		size_t playerCount(0ull);
		for each (const auto & componentParam in components) {
			auto* spawnComponent = static_cast<PlayerSpawn_Component*>(componentParam[0]);
			auto* transformComponent = static_cast<Transform_Component*>(componentParam[1]);

			auto& transform = transformComponent->m_worldTransform;
			if (playerCount == 0ull) {
				Player3D_Component player;
				Transform_Component trans;

				trans.m_localTransform = transform;

				ecsBaseComponent* entityComponents[] = { &player, &trans };
				m_game->getWorld().makeEntity(entityComponents, 2ull, "Player");
			}
		}
	};


private:
	// Private Attributes
	Engine* m_engine = nullptr;
	Game_Module* m_game = nullptr;
	size_t m_playerCount = 0ull;
	std::shared_ptr<bool> m_aliveIndicator = std::make_shared<bool>(true);
};

#endif // PLAYERSPAWNER_SYSTEM_H