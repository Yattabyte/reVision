#include "Modules/StartScreen/StartScreen_M.h"
#include "Modules/UI/Macro Elements/StartMenu.h"
#include "Modules/UI/Macro Elements/PauseMenu.h"
#include "Engine.h"


void StartScreen_Module::initialize(Engine* engine)
{
	Engine_Module::initialize(engine);
	m_engine->getManager_Messages().statement("Loading Module: Starting Screen...");

	// Create Main Menu
	auto startMenu = std::make_shared<StartMenu>(m_engine);
	m_startMenu = startMenu;
	startMenu->addCallback(StartMenu::on_start_game, [&]() {
		m_engine->goToGame();
		});
	startMenu->addCallback(StartMenu::on_level_editor, [&]() {
		m_engine->goToEditor();
		});
	startMenu->addCallback(StartMenu::on_quit, [&]() {
		//
		});
}

void StartScreen_Module::deinitialize()
{
	m_engine->getManager_Messages().statement("Unloading Module: Starting Screen...");
}

void StartScreen_Module::showStartMenu()
{
	m_engine->setMouseInputMode(Engine::MouseInputMode::NORMAL);
	m_engine->getModule_UI().clear();
	m_engine->getModule_UI().pushRootElement(m_startMenu);
	m_engine->getModule_UI().setFocusMap(std::dynamic_pointer_cast<StartMenu>(m_startMenu)->getFocusMap());
}