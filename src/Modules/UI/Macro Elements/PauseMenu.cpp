#include "Modules/UI/Macro Elements/PauseMenu.h"
#include "Engine.h"


PauseMenu::PauseMenu(Engine& engine) noexcept :
	Menu(engine)
{
	// Title
	m_title->setText("PAUSE MENU");

	// Add 'Start Game' button
	addButton(engine, "RESUME", [&]() noexcept { resume(); });

	// Add 'Options' button
	m_optionsMenu = std::make_shared<OptionsMenu>(engine);
	addButton(engine, "  OPTIONS >", [&]() noexcept { goToOptions(); });
	m_optionsMenu->addCallback((int)OptionsMenu::Interact::on_back, [&]() noexcept { returnFromOptions(); });

	// Add 'Quit' button
	addButton(engine, "END GAME", [&]() noexcept { quit(); });

	// Callbacks
	addCallback((int)UI_Element::Interact::on_resize, [&]() noexcept {
		const auto scale = getScale();
		m_optionsMenu->setScale(scale);
		});

	// Populate Focus Map
	m_focusMap = std::make_shared<FocusMap>();
	m_focusMap->addElement(m_layout);
	engine.getModule_UI().setFocusMap(getFocusMap());
}

void PauseMenu::resume() noexcept
{
	enactCallback((int)PauseMenu::Interact::on_resume_game);
}

void PauseMenu::goToOptions() noexcept 
{
	// Transfer appearance and control to options menu
	auto& ui = m_engine.getModule_UI();
	ui.pushRootElement(m_optionsMenu);
	ui.setFocusMap(m_optionsMenu->getFocusMap());
	m_layout->setSelectionIndex(-1);
	enactCallback((int)PauseMenu::Interact::on_options);
}

void PauseMenu::returnFromOptions() noexcept
{
	// Transfer control back to this menu
	m_engine.getModule_UI().setFocusMap(getFocusMap());
}

void PauseMenu::quit() noexcept 
{
	m_engine.getModule_UI().clear();
	enactCallback((int)PauseMenu::Interact::on_end);
}