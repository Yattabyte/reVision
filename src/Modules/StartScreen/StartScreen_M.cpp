#include "Modules/StartScreen/StartScreen_M.h"
#include "Modules/UI/Macro Elements/StartMenu.h"
#include "Engine.h"


StartScreen_Module::StartScreen_Module(Engine& engine) noexcept : 
	Engine_Module(engine) 
{
}

void StartScreen_Module::initialize()
{
	Engine_Module::initialize();
	m_engine.getManager_Messages().statement("Loading Module: Starting Screen...");

	// Create Main Menu
	auto startMenu = std::make_shared<StartMenu>(m_engine);
	m_startMenu = startMenu;
	startMenu->addCallback((int)StartMenu::Interact::on_start_game, [&]() noexcept {
		m_engine.goToGame();
		});
	startMenu->addCallback((int)StartMenu::Interact::on_level_editor, [&]() noexcept {
		m_engine.goToEditor();
		});
	startMenu->addCallback((int)StartMenu::Interact::on_quit, [&]() noexcept {
		//
		});
}

void StartScreen_Module::deinitialize()
{
	m_engine.getManager_Messages().statement("Unloading Module: Starting Screen...");

	m_startMenu.reset();
}

void StartScreen_Module::frameTick(const float& deltaTime)
{
	m_engine.getModule_Physics().frameTick(m_world, deltaTime);
	m_engine.getModule_Graphics().renderWorld(m_world, deltaTime);
}

void StartScreen_Module::showStartMenu()
{
	m_engine.setMouseInputMode(Engine::MouseInputMode::NORMAL);
	m_engine.getModule_UI().clear();
	m_engine.getModule_UI().pushRootElement(m_startMenu);
	m_engine.getModule_UI().setFocusMap(std::dynamic_pointer_cast<StartMenu>(m_startMenu)->getFocusMap());
}