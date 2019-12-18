#include "Modules/Game/ECS/PlayerSpawner_System.h"
#include "Modules/Game/Game_M.h"
#include "Modules/ECS/component_types.h"
#include "Engine.h"


PlayerSpawn_System::PlayerSpawn_System(Engine& engine, Game_Module& game)
	: m_engine(engine), m_game(game) 
{
	// Declare component types used
	addComponentType(PlayerSpawn_Component::Runtime_ID, RequirementsFlag::FLAG_REQUIRED);
	addComponentType(Transform_Component::Runtime_ID, RequirementsFlag::FLAG_REQUIRED);

	// Error Reporting
	if (!isValid())
		engine.getManager_Messages().error("Invalid ECS System: PlayerSpawn_System");
}

void PlayerSpawn_System::updateComponents(const float&, const std::vector<std::vector<ecsBaseComponent*>>& components) 
{
	if (m_playerCount == 0ull) {
		for (const auto& componentParam : components) {
			//auto* spawnComponent = static_cast<PlayerSpawn_Component*>(componentParam[0]);
			const auto* transformComponent = static_cast<Transform_Component*>(componentParam[1]);
			Player3D_Component player;
			Transform_Component trans;

			trans.m_localTransform = transformComponent->m_worldTransform;

			const ecsBaseComponent* const entityComponents[] = { &player, &trans };
			m_playerHandle = m_game.getWorld().makeEntity(entityComponents, 2ull, "Player");
			m_playerCount++;
		}
	}
}