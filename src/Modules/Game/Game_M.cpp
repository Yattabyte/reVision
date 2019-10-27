#include "Modules/Game/Game_M.h"
#include "Modules/Game/ECS/PlayerSpawner_System.h"
#include "Modules/Game/ECS/PlayerFreeLook_System.h"
#include "Modules/Game/Overlays/LoadingIndicator.h"
#include "Modules/Game/Overlays/Frametime_Counter.h"
#include "Modules/UI/Macro Elements/StartMenu.h"
#include "Modules/UI/Macro Elements/PauseMenu.h"
#include "Engine.h"


void Game_Module::initialize(Engine* engine)
{
	Engine_Module::initialize(engine);
	m_engine->getManager_Messages().statement("Loading Module: Game...");

	// Initialize ECS Systems
	m_Systems.makeSystem<PlayerSpawn_System>(engine, this);
	m_Systems.makeSystem<PlayerFreeLook_System>(engine);

	// Create Overlay Effects
	m_loadingRing = std::make_shared<LoadingIndicator>(engine);
	m_frameTime = std::make_shared<Frametime_Counter>(engine);

	// Create Pause Menu
	auto pauseMenu = std::make_shared<PauseMenu>(engine);
	m_pauseMenu = pauseMenu;
	pauseMenu->addCallback(PauseMenu::on_resume_game, [&]() {
		showPauseMenu(false);
		pauseMenu->setVisible(true);
		});
	pauseMenu->addCallback(PauseMenu::on_end, [&]() {
		showPauseMenu(false);
		m_engine->goToMainMenu();
		});
}

void Game_Module::deinitialize()
{
	m_engine->getManager_Messages().statement("Unloading Module: Game...");
}

void Game_Module::frameTick(const float& deltaTime)
{
	auto& actionState = m_engine->getActionState();
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
	m_world.updateSystems(m_Systems, deltaTime);
	m_engine->getModule_Physics().frameTick(m_world, deltaTime);
	m_engine->getModule_Graphics().renderWorld(m_world, deltaTime);
	renderOverlays(deltaTime);
}

ecsWorld& Game_Module::getWorld()
{
	return m_world;
}

void Game_Module::renderOverlays(const float& deltaTime)
{
	m_loadingRing->applyEffect(deltaTime);
	m_frameTime->applyEffect(deltaTime);
}

void Game_Module::showGame()
{
	m_gameState = in_game;
	//m_engine->getModule_World().loadWorld("a.bmap");
	m_engine->setMouseInputMode(Engine::MouseInputMode::FREE_LOOK);
}

void Game_Module::showPauseMenu(const bool& show)
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