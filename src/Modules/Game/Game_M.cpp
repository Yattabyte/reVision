#include "Modules/Game/Game_M.h"
#include "Modules/Game/ECS/components.h"
#include "Modules/Game/ECS/PlayerFreeLook_S.h"
#include "Modules/Game/Overlays/LoadingIndicator.h"
#include "Modules/Game/Overlays/Frametime_Counter.h"
#include "Modules/UI/Macro Elements/StartMenu.h"
#include "Modules/UI/Macro Elements/PauseMenu.h"
#include "Engine.h"


Game_Module::~Game_Module()
{
	// Remove support for the following list of component types
	auto & world = m_engine->getModule_World();
	world.removeComponentType("Player3D_Component");
	world.removeComponentType("MenuCamera_Component");
}

void Game_Module::initialize(Engine * engine)
{
	Engine_Module::initialize(engine);
	m_engine->getManager_Messages().statement("Loading Module: Game...");

	// Initialize ECS Systems
	m_ecsSystems.addSystem(new PlayerFreeLook_System(m_engine));

	// Add support for the following list of component types
	auto & world = m_engine->getModule_World();
	world.addComponentType("Player3D_Component", [](const ParamList & parameters) {
		auto * component = new Player3D_Component();
		return std::make_pair(component->ID, component);
	});
	world.addComponentType("MenuCamera_Component", [](const ParamList & parameters) {
		auto * component = new Player3D_Component();
		return std::make_pair(component->ID, component);
	});

	// Create Overlay Effects
	m_loadingRing = std::make_shared<LoadingIndicator>(m_engine);
	m_frameTime = std::make_shared<Frametime_Counter>(m_engine);

	// Create Main Menu
	auto startMenu = std::make_shared<StartMenu>(m_engine);
	m_startMenu = startMenu;
	startMenu->addCallback(StartMenu::on_start_game, [&]() {
		startGame();
	});

	// Create Pause Menu
	auto pauseMenu = std::make_shared<PauseMenu>(m_engine);
	m_pauseMenu = pauseMenu;
	pauseMenu->addCallback(PauseMenu::on_resume_game, [&]() {
		showPauseMenu(false);
		pauseMenu->setVisible(true);
	});
	pauseMenu->addCallback(PauseMenu::on_end, [&]() {
		showPauseMenu(false);
		showStartMenu();
	});

	showStartMenu();
}

void Game_Module::frameTick(const float & deltaTime)
{
	auto & actionState = m_engine->getActionState();
	if (m_gameState == in_pauseMenu || m_gameState == in_game) {
		// Check if we should show the overlay
		if (actionState.isAction(ActionState::UI_ESCAPE) == ActionState::PRESS)
			showPauseMenu(m_gameState == in_game ? true : false);

		// Handle GLOBAL user input
		if (m_gameState == in_game) {
			if (m_engine->getMouseInputMode() == Engine::FREE_LOOK) {
				actionState[ActionState::LOOK_X] = actionState[ActionState::MOUSE_X];
				actionState[ActionState::LOOK_Y] = actionState[ActionState::MOUSE_Y];
				actionState[ActionState::FIRE1] = actionState[ActionState::MOUSE_L];
				actionState[ActionState::FIRE2] = actionState[ActionState::MOUSE_R];
			}
		}
	}

	// Update our own ECS systems
	m_engine->getModule_World().updateSystems(m_ecsSystems, deltaTime);
}

void Game_Module::renderOverlays(const float & deltaTime)
{
	m_loadingRing->applyEffect(deltaTime);
	m_frameTime->applyEffect(deltaTime);
}

void Game_Module::showStartMenu()
{
	m_engine->setMouseInputMode(Engine::MouseInputMode::NORMAL);
	m_engine->getModule_UI().clear();
	m_engine->getModule_UI().pushRootElement(m_startMenu);
	m_engine->getModule_UI().setFocusMap(std::dynamic_pointer_cast<StartMenu>(m_startMenu)->getFocusMap());
	m_gameState = in_startMenu;

	m_engine->getModule_World().loadWorld("background.map");
}

void Game_Module::showPauseMenu(const bool & show)
{
	if (show) {
		m_engine->setMouseInputMode(Engine::MouseInputMode::NORMAL);
		m_engine->getModule_UI().clear();
		m_engine->getModule_UI().pushRootElement(m_pauseMenu);
		m_engine->getModule_UI().setFocusMap(std::dynamic_pointer_cast<PauseMenu>(m_pauseMenu)->getFocusMap());
		m_gameState = in_pauseMenu;
	}
	else {
		m_engine->setMouseInputMode(Engine::MouseInputMode::FREE_LOOK);
		m_engine->getModule_UI().clear();
		m_gameState = in_game;
	}
}

void Game_Module::startGame()
{
	m_gameState = in_game;
	m_engine->getModule_World().loadWorld("Lighting Tests//Directional_Test.map");
	m_engine->setMouseInputMode(Engine::MouseInputMode::FREE_LOOK);
}
