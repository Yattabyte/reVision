#include "Modules/UI/Macro Elements/StartMenu.h"
#include "Modules/UI/FocusMap.h"
#include "Engine.h"


StartMenu::StartMenu(Engine& engine) : Menu(engine)
{
	// Title
	m_title->setText("MAIN MENU");

	// Add 'Start Game' button
	m_gameMenu = std::make_shared<GameMenu>(engine);
	addButton(engine, "START GAME", [&] { startGame(); });
	m_gameMenu->addCallback(static_cast<int>(GameMenu::Interact::on_back), [&]() noexcept { m_engine.getModule_UI().setFocusMap(getFocusMap()); });
	m_gameMenu->addCallback(static_cast<int>(GameMenu::Interact::on_levelSelect), [&]() noexcept {
		m_engine.getModule_UI().clear();
		m_engine.getModule_Game().loadLevel(m_gameMenu->getLevel());
		m_engine.goToGame();
		enactCallback(static_cast<int>(StartMenu::Interact::on_start_game));
		});

	// Add 'Level Editor' button
	addButton(engine, "LEVEL EDITOR", [&] { startEditor(); });

	// Add 'Options' button
	m_optionsMenu = std::make_shared<OptionsMenu>(engine);
	addButton(engine, "  OPTIONS >", [&] { goToOptions(); });
	m_optionsMenu->addCallback(static_cast<int>(OptionsMenu::Interact::on_back), [&]() noexcept {m_engine.getModule_UI().setFocusMap(getFocusMap()); });

	// Add 'Quit' button
	addButton(engine, "QUIT", [&] { quit(); });

	// Callbacks
	addCallback(static_cast<int>(UI_Element::Interact::on_resize), [&] {
		const auto scale = getScale();
		m_optionsMenu->setScale(scale);
		});

	// Populate Focus Map
	m_focusMap = std::make_shared<FocusMap>();
	m_focusMap->addElement(m_layout);
	m_focusMap->focusElement(m_layout);
	engine.getModule_UI().setFocusMap(getFocusMap());
}

void StartMenu::startGame()
{
	// Transfer appearance and control to game menu
	auto& ui = m_engine.getModule_UI();
	ui.pushRootElement(m_gameMenu);
	ui.setFocusMap(m_gameMenu->getFocusMap());
	m_layout->setSelectionIndex(-1);
}

void StartMenu::startEditor()
{
	m_engine.getModule_UI().clear();
	m_engine.goToEditor();
	enactCallback(static_cast<int>(StartMenu::Interact::on_level_editor));
}

void StartMenu::goToOptions() 
{
	// Transfer appearance and control to options menu
	auto& ui = m_engine.getModule_UI();
	ui.pushRootElement(m_optionsMenu);
	ui.setFocusMap(m_optionsMenu->getFocusMap());
	m_layout->setSelectionIndex(-1);
	enactCallback(static_cast<int>(StartMenu::Interact::on_options));
}

void StartMenu::quit() 
{
	m_engine.getModule_UI().clear();
	m_engine.shutDown();
	enactCallback(static_cast<int>(StartMenu::Interact::on_quit));
}