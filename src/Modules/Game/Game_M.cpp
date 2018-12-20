#include "Modules\Game\Game_M.h"
#include "Assets\Asset_Image.h"
#include "Engine.h"
#include <atomic>

/* Component Types Used */
#include "Modules\Game\Components\Board_C.h"
#include "Modules\Game\Components\Score_C.h"
#include "Modules\Game\Components\Player3D_C.h"

/* Game System Types Used */
#include "Modules\Game\Systems\Board_S.h"
#include "Modules\Game\Systems\Gravity_S.h"
#include "Modules\Game\Systems\PlayerInput_S.h"
#include "Modules\Game\Systems\PlayerFreeLook_S.h"
#include "Modules\Game\Systems\Push_S.h"
#include "Modules\Game\Systems\Score_S.h"
#include "Modules\Game\Systems\Timer_S.h"
#include "Modules\Game\Systems\Music.h"

/* Rendering System Types Used */
#include "Modules\Game\Systems\Rendering_S.h"


void Game_Module::initialize(Engine * engine)
{
	Engine_Module::initialize(engine);
	m_engine->getMessageManager().statement("Loading Module: Game...");

	// Gameplay Systems
	m_gameplaySystems = {
		new Board_System(),
		new Gravity_System(m_engine),
		new Push_System(),
		new PlayerInput_System(m_engine, &m_engine->getActionState()),
		new Score_System(m_engine),
		new Timer_System(),
		new Music_System(m_engine),
		//new PlayerFreeLook_System(engine)
	};
	for each (auto * system in m_gameplaySystems)
		m_systemList.addSystem(system);

	// Rendering Systems
	m_renderingSystem = new Rendering_System(m_engine, m_engine->getGraphicsModule().getLightingFBOID());

	// Create Players
	m_players.push_back(m_boardBuffer.newElement());

	// Component Constructors
	m_engine->registerECSConstructor("Board_Component", new Board_Constructor(m_players[0]));
	m_engine->registerECSConstructor("Score_Component", new Score_Constructor(m_players[0]));
	m_engine->registerECSConstructor("Player3D_Component", new Player3D_Constructor());
}

void Game_Module::frameTick(const float & deltaTime)
{	
	// Check if safe to start game
	if (!m_readyToStart) {
		bool allReady = true;
		for each (auto * system in m_gameplaySystems)
			if (!system->readyToUse()) {
				allReady = false;
				break;
			}
		if (allReady)
			m_readyToStart = true;
	}

	// Update Game
	if (m_readyToStart) {
		m_timeAccumulator += deltaTime;
		auto & ecs = m_engine->getECS();
		constexpr float dt = 0.01f;
		while (m_timeAccumulator >= dt) {
			// Update ALL systems with our fixed tick rate
			ecs.updateSystems(m_systemList, deltaTime);
			m_timeAccumulator -= dt;
		}

		// Render Game
		m_boardBuffer.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 8);
		ecs.updateSystem(m_renderingSystem, deltaTime);
	}
}
