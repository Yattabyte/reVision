#include "Modules/UI/Macro Elements/PauseMenu.h"
#include "Engine.h"


PauseMenu::PauseMenu(Engine& engine) :
	Menu(engine)
{
	// Title
	m_title->setText("PAUSE MENU");

	// Add 'Start Game' button
	addButton(engine, "RESUME", [&] { resume(); });

	// Add 'Options' button
	m_optionsMenu = std::make_shared<OptionsMenu>(engine);
	addButton(engine, "  OPTIONS >", [&] { goToOptions(); });
	m_optionsMenu->addCallback(static_cast<int>(OptionsMenu::Interact::on_back), [&]() noexcept { returnFromOptions(); });

	// Add 'Quit' button
	addButton(engine, "END GAME", [&] { quit(); });

	// Callbacks
	addCallback(static_cast<int>(UI_Element::Interact::on_resize), [&] {
		const auto scale = getScale();
		m_optionsMenu->setScale(scale);
		});

	// Populate Focus Map
	m_focusMap = std::make_shared<FocusMap>();
	m_focusMap->addElement(m_layout);
	engine.getModule_UI().setFocusMap(getFocusMap());
}

void PauseMenu::resume()
{
	enactCallback(static_cast<int>(PauseMenu::Interact::on_resume_game));
}

void PauseMenu::goToOptions() 
{
	// Transfer appearance and control to options menu
	auto& ui = m_engine.getModule_UI();
	ui.pushRootElement(m_optionsMenu);
	ui.setFocusMap(m_optionsMenu->getFocusMap());
	m_layout->setSelectionIndex(-1);
	enactCallback(static_cast<int>(PauseMenu::Interact::on_options));
}

void PauseMenu::returnFromOptions() noexcept
{
	// Transfer control back to this menu
	m_engine.getModule_UI().setFocusMap(getFocusMap());
}

void PauseMenu::quit() 
{
	m_engine.getModule_UI().clear();
	enactCallback(static_cast<int>(PauseMenu::Interact::on_end));
}