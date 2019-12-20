#include "Modules/Game/Game_M.h"
#include "Modules/Game/ECS/PlayerSpawner_System.h"
#include "Modules/Game/ECS/PlayerFreeLook_System.h"
#include "Modules/UI/Macro Elements/StartMenu.h"
#include "Modules/UI/Macro Elements/PauseMenu.h"
#include "Utilities/IO/Level_IO.h"
#include "Engine.h"


Game_Module::Game_Module(Engine& engine) : 
	Engine_Module(engine), 
	m_loadingRing(engine),
	m_frameTime(engine)
{
}

void Game_Module::initialize()
{
	Engine_Module::initialize();
	m_engine.getManager_Messages().statement("Loading Module: Game...");

	// Initialize ECS Systems
	m_Systems.makeSystem<PlayerSpawn_System>(m_engine, *this);
	m_Systems.makeSystem<PlayerFreeLook_System>(m_engine);

	// Create Pause Menu
	auto pauseMenu = std::make_shared<PauseMenu>(m_engine);
	m_pauseMenu = pauseMenu;
	pauseMenu->addCallback((int)PauseMenu::Interact::on_resume_game, [&]() noexcept {
		showPauseMenu(false);
		pauseMenu->setVisible(true);
		});
	pauseMenu->addCallback((int)PauseMenu::Interact::on_end, [&]() noexcept {
		showPauseMenu(false);
		m_engine.goToMainMenu();
		});
}

void Game_Module::deinitialize()
{
	m_engine.getManager_Messages().statement("Unloading Module: Game...");
}

void Game_Module::frameTick(const float& deltaTime)
{
	auto& actionState = m_engine.getActionState();
	if (m_gameState == Game_State::in_pauseMenu || m_gameState == Game_State::in_game) {
		// Check if we should show the overlay
		if (actionState.isAction(ActionState::Action::UI_ESCAPE) == ActionState::State::PRESS)
			showPauseMenu(m_gameState == Game_State::in_game);

		// Handle GLOBAL user input
		if (m_gameState == Game_State::in_game) {
			if (m_engine.getMouseInputMode() == Engine::MouseInputMode::FREE_LOOK) {
				actionState[ActionState::Action::LOOK_X] = actionState[ActionState::Action::MOUSE_X];
				actionState[ActionState::Action::LOOK_Y] = actionState[ActionState::Action::MOUSE_Y];
				actionState[ActionState::Action::FIRE1] = actionState[ActionState::Action::MOUSE_L];
				actionState[ActionState::Action::FIRE2] = actionState[ActionState::Action::MOUSE_R];
			}
		}
	}

	// Update our own ECS systems
	m_world.updateSystems(m_Systems, deltaTime);
	m_engine.getModule_Physics().frameTick(m_world, deltaTime);
	m_engine.getModule_Graphics().renderWorld(m_world, deltaTime);
	renderOverlays(deltaTime);
}

ecsWorld& Game_Module::getWorld() noexcept
{
	return m_world;
}

void Game_Module::renderOverlays(const float& deltaTime)
{
	m_loadingRing.applyEffect(deltaTime);
	m_frameTime.applyEffect(deltaTime);
}

void Game_Module::showGame()
{
	m_gameState = Game_State::in_game;
	m_engine.setMouseInputMode(Engine::MouseInputMode::FREE_LOOK);
	if (!Level_IO::Import_BMap("Phys Test.bmap", m_world))
		m_engine.getManager_Messages().error("Cannot open the level: Phys Test.bmap");
}

void Game_Module::showPauseMenu(const bool& show)
{
	if (show) {
		m_engine.setMouseInputMode(Engine::MouseInputMode::NORMAL);
		m_engine.getModule_UI().clear();
		m_engine.getModule_UI().pushRootElement(m_pauseMenu);
		m_engine.getModule_UI().setFocusMap(std::dynamic_pointer_cast<PauseMenu>(m_pauseMenu)->getFocusMap());
		m_gameState = Game_State::in_pauseMenu;
	}
	else {
		m_engine.setMouseInputMode(Engine::MouseInputMode::FREE_LOOK);
		m_engine.getModule_UI().clear();
		m_gameState = Game_State::in_game;
	}
}