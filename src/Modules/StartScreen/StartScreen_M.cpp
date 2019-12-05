#include "Modules/StartScreen/StartScreen_M.h"
#include "Modules/UI/Macro Elements/StartMenu.h"
#include "Engine.h"


StartScreen_Module::StartScreen_Module(Engine& engine) : 
	Engine_Module(engine) 
{
}

void StartScreen_Module::initialize() noexcept
{
	Engine_Module::initialize();
	m_engine.getManager_Messages().statement("Loading Module: Starting Screen...");

	// Create Main Menu
	auto startMenu = std::make_shared<StartMenu>(m_engine);
	m_startMenu = startMenu;
	startMenu->addCallback((int)StartMenu::Interact::on_start_game, [&]() {
		m_engine.goToGame();
		});
	startMenu->addCallback((int)StartMenu::Interact::on_level_editor, [&]() {
		m_engine.goToEditor();
		});
	startMenu->addCallback((int)StartMenu::Interact::on_quit, [&]() {
		//
		});
}

void StartScreen_Module::deinitialize() noexcept
{
	m_engine.getManager_Messages().statement("Unloading Module: Starting Screen...");

	m_startMenu.reset();
}

void StartScreen_Module::frameTick(const float& deltaTime) noexcept
{
	m_engine.getModule_Physics().frameTick(m_world, deltaTime);
	m_engine.getModule_Graphics().renderWorld(m_world, deltaTime);
}

void StartScreen_Module::showStartMenu() noexcept
{
	m_engine.setMouseInputMode(Engine::MouseInputMode::NORMAL);
	m_engine.getModule_UI().clear();
	m_engine.getModule_UI().pushRootElement(m_startMenu);
	m_engine.getModule_UI().setFocusMap(std::dynamic_pointer_cast<StartMenu>(m_startMenu)->getFocusMap());
}