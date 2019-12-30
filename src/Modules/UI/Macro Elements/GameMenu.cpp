#include "Modules/UI/Macro Elements/GameMenu.h"
#include "Engine.h"


GameMenu::GameMenu(Engine& engine) :
	Menu(engine)
{
	// Title
	m_title->setText("Game Options");

	// Add 'Levels' button
	m_mapMenu = std::make_shared<Options_Levels>(engine);
	m_mapMenu->setVisible(false);
	m_mapMenu->addCallback(static_cast<int>(Options_Levels::Interact::on_levelSelect), [&]() noexcept { enactCallback(static_cast<int>(GameMenu::Interact::on_levelSelect)); });
	addButton(engine, "Levels", [&] { levelSelect(); });
	addElement(m_mapMenu);

	// Add 'Back' button
	addButton(engine, "< BACK  ", [&] { back(); });

	// Callbacks
	addCallback(static_cast<int>(UI_Element::Interact::on_resize), [&] {
		const auto scale = getScale();
		m_mapMenu->setScale({ (scale.x / 2.0f) - 320.0f, scale.y / 2.0f });
		m_mapMenu->setPosition({ (scale.x / 2.0f) + 192.0f, scale.y / 2.0f });
	});

	m_focusMap = std::make_shared<FocusMap>();
	m_focusMap->addElement(m_layout);
	m_focusMap->focusElement(m_layout);
}

std::string GameMenu::getLevel() const
{
	return m_mapMenu->m_level;
}

void GameMenu::levelSelect()
{
	// Remove control from the graphics menu
	m_mapMenu->setVisible(false);
	m_focusMap->clear();

	// Transfer control to the video menu
	m_mapMenu->setVisible(true);
	m_focusMap->addElement(m_layout);
	m_focusMap->addElement(m_mapMenu);
	m_focusMap->focusElement(m_mapMenu);
}

void GameMenu::back()
{
	m_mapMenu->setVisible(false);
	m_focusMap->clear();
	m_engine.getPreferenceState().save();

	// Revert appearance and control back to previous element (start menu, pause menu, etc)
	m_engine.getModule_UI().popRootElement();
	m_layout->setSelectionIndex(-1);
	enactCallback(static_cast<int>(GameMenu::Interact::on_back));
}