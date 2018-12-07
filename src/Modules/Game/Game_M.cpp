#include "Modules\Game\Game_M.h"
#include "Assets\Asset_Image.h"
#include "Engine.h"
#include <atomic>

/* Component Types Used */
#include "Modules\Game\Components\GameBoard_C.h"
#include "Modules\Game\Components\GameScore_C.h"
#include "Modules\Game\Components\Player_C.h"

/* Game System Types Used */
#include "Modules\Game\Systems\Board_S.h"
#include "Modules\Game\Systems\Gravity_S.h"
#include "Modules\Game\Systems\PlayerInput_S.h"
#include "Modules\Game\Systems\PlayerFreeLook_S.h"
#include "Modules\Game\Systems\Push_S.h"
#include "Modules\Game\Systems\Score_S.h"
#include "Modules\Game\Systems\Timer_S.h"

/* Rendering System Types Used */
#include "Modules\Game\Systems\Rendering_S.h"


void Game_Module::initialize(Engine * engine)
{
	Engine_Module::initialize(engine);
	m_engine->getMessageManager().statement("Loading Module: Game...");

	// Gameplay Systems
	m_gameplaySystems.addSystem(new Board_System());
	m_gameplaySystems.addSystem(new Gravity_System());
	m_gameplaySystems.addSystem(new Push_System());
	m_gameplaySystems.addSystem(new PlayerInput_System(&m_engine->getActionState()));
	m_gameplaySystems.addSystem(new Score_System(m_engine));
	m_gameplaySystems.addSystem(new Timer_System());
	//m_gameplaySystems.addSystem(new PlayerFreeLook_System(engine));

	// Rendering Systems
	m_renderingSystem = new Rendering_System(m_engine, m_engine->getGraphicsModule().getLightingFBOID());

	// Component Constructors
	m_engine->registerECSConstructor("GameBoard_Component", new GameBoard_Constructor(&m_engine->getGameModule().m_boardBuffer));
	m_engine->registerECSConstructor("GameScore_Component", new GameScore_Constructor());
	m_engine->registerECSConstructor("Player_Component", new Player_Constructor());
}

void Game_Module::frameTick(const float & deltaTime)
{	
	// Update Game
	m_timeAccumulator += deltaTime;
	auto & ecs = m_engine->getECS();
	constexpr float dt = 0.01f;
	while (m_timeAccumulator >= dt) {
		// Update ALL systems with our fixed tick rate
		ecs.updateSystems(m_gameplaySystems, deltaTime);
		m_timeAccumulator -= dt;
	}

	// Render Game
	m_boardBuffer.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 8);
	ecs.updateSystem(m_renderingSystem, deltaTime);	
}
