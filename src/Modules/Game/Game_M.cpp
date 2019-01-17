#include "Modules/Game/Game_M.h"
#include "Assets/Asset_Image.h"
#include "Engine.h"
#include <atomic>

/* Component Types Used */
#include "Modules/Game/Components/Board_C.h"
#include "Modules/Game/Components/Score_C.h"
#include "Modules/Game/Components/Player3D_C.h"

/* Game System Types Used */
#include "Modules/Game/Systems/IntroOutro_S.h"
#include "Modules/Game/Systems/ColorScheme_S.h"
#include "Modules/Game/Systems/Gravity_S.h"
#include "Modules/Game/Systems/PlayerInput_S.h"
#include "Modules/Game/Systems/PlayerFreeLook_S.h"
#include "Modules/Game/Systems/Push_S.h"
#include "Modules/Game/Systems/Score_S.h"
#include "Modules/Game/Systems/Timer_S.h"
#include "Modules/Game/Systems/Music.h"

/* Rendering System Types Used */
#include "Modules/Game/Systems/Rendering_S.h"


void Game_Module::initialize(Engine * engine)
{
	Engine_Module::initialize(engine);
	m_engine->getManager_Messages().statement("Loading Module: Game...");

	// Gameplay Systems
	m_gameplaySystems = {
		new IntroOutro_System(m_engine),
		new ColorScheme_System(),
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
	m_renderingSystem = new Rendering_System(m_engine, m_engine->getModule_Graphics().getLightingFBOID());

	// Create Players
	auto & gameBoard = *m_boardBuffer.newElement();
	(*gameBoard.data) = GameBuffer();
	// Reset game buffer data
	for (int x = 0; x < BOARD_WIDTH; ++x) {
		for (int y = 0; y < BOARD_HEIGHT; ++y) {
			gameBoard.data->tiles[(y * 6) + x].type = TileState::NONE;
			gameBoard.data->tiles[(y * 6) + x].yOffset = 0.0f;
			gameBoard.data->tiles[(y * 6) + x].lifeLinear = 0.0f;
		}
		gameBoard.data->lanes[x] = 0.0f;
	}
	m_players.push_back(&gameBoard);

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
	if (m_readyToStart && m_userReady) {
		m_timeAccumulator += deltaTime;
		auto & ecs = m_engine->getECS();
		constexpr float dt = 1.0f / 60.0f;
		while (m_timeAccumulator >= dt) {
			// Update ALL systems using a fixed tick rate
			ecs.updateSystems(m_systemList, dt);
			m_timeAccumulator -= dt;
		}

		// Render Game
		m_boardBuffer.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 8);
		ecs.updateSystem(m_renderingSystem, deltaTime);
	}
}

void Game_Module::startGame()
{
	m_userReady = true;
}
