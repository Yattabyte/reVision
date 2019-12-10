#include "Modules/UI/Macro Elements/StartMenu.h"
#include "Modules/UI/FocusMap.h"
#include "Engine.h"


StartMenu::StartMenu(Engine& engine) noexcept : Menu(engine)
{
	// Title
	m_title->setText("MAIN MENU");

	// Add 'Start Game' button
	addButton(engine, "START GAME", [&]() noexcept { startGame(); });

	// Add 'Level Editor' button
	addButton(engine, "LEVEL EDITOR", [&]() noexcept { startEditor(); });

	// Add 'Options' button
	m_optionsMenu = std::make_shared<OptionsMenu>(engine);
	addButton(engine, "  OPTIONS >", [&]() noexcept { goToOptions(); });
	m_optionsMenu->addCallback((int)OptionsMenu::Interact::on_back, [&]() noexcept { returnFromOptions(); });

	// Add 'Quit' button
	addButton(engine, "QUIT", [&]() noexcept { quit(); });

	// Callbacks
	addCallback((int)UI_Element::Interact::on_resize, [&]() noexcept {
		const auto scale = getScale();
		m_optionsMenu->setScale(scale);
		});

	// Populate Focus Map
	m_focusMap = std::make_shared<FocusMap>();
	m_focusMap->addElement(m_layout);
	m_focusMap->focusElement(m_layout);
	engine.getModule_UI().setFocusMap(getFocusMap());
}

void StartMenu::startGame() noexcept
{
	m_engine.getModule_UI().clear();
	enactCallback((int)StartMenu::Interact::on_start_game);
}

void StartMenu::startEditor() noexcept
{
	m_engine.getModule_UI().clear();
	enactCallback((int)StartMenu::Interact::on_level_editor);
}

void StartMenu::goToOptions() noexcept 
{
	// Transfer appearance and control to options menu
	auto& ui = m_engine.getModule_UI();
	ui.pushRootElement(m_optionsMenu);
	ui.setFocusMap(m_optionsMenu->getFocusMap());
	m_layout->setSelectionIndex(-1);
	enactCallback((int)StartMenu::Interact::on_options);
}

void StartMenu::returnFromOptions() noexcept 
{
	// Transfer control back to this menu
	m_engine.getModule_UI().setFocusMap(getFocusMap());
}

void StartMenu::quit() noexcept 
{
	m_engine.getModule_UI().clear();
	m_engine.shutDown();
	enactCallback((int)StartMenu::Interact::on_quit);
}