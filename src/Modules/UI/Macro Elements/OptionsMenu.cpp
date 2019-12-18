#include "Modules/UI/Macro Elements/OptionsMenu.h"
#include "Engine.h"


OptionsMenu::OptionsMenu(Engine& engine) : 
	Menu(engine) 
{
	// Title
	m_title->setText("OPTIONS");

	// Add 'Video' button
	m_videoMenu = std::make_shared<Options_Video>(engine);
	m_videoMenu->setVisible(false);
	addButton(engine, "VIDEO", [&]() noexcept { video(); });
	addElement(m_videoMenu);

	// Add 'Graphics' button
	m_gfxMenu = std::make_shared<Options_Graphics>(engine);
	m_gfxMenu->setVisible(false);
	addButton(engine, "GRAPHICS", [&]() noexcept { graphics(); });
	addElement(m_gfxMenu);

	// Add 'Controls' button
	addButton(engine, "CONTROLS", [&]() noexcept { controls(); });

	// Add 'Back' button
	addButton(engine, "< BACK  ", [&]() noexcept { back(); });

	// Callbacks
	addCallback((int)UI_Element::Interact::on_resize, [&]() noexcept {
		const auto scale = getScale();
		m_videoMenu->setScale({ (scale.x / 2.0f) - 320.0f, scale.y / 2.0f });
		m_gfxMenu->setScale({ (scale.x / 2.0f) - 320.0f, scale.y / 2.0f });
		m_videoMenu->setPosition({ (scale.x / 2.0f) + 192.0f, scale.y / 2.0f });
		m_gfxMenu->setPosition({ (scale.x / 2.0f) + 192.0f, scale.y / 2.0f });
		});

	m_focusMap = std::make_shared<FocusMap>();
	m_focusMap->addElement(m_layout);
	m_focusMap->focusElement(m_layout);
}

void OptionsMenu::video() 
{
	// Remove control from the graphics menu
	m_gfxMenu->setVisible(false);
	m_focusMap->clear();

	// Transfer control to the video menu
	m_videoMenu->setVisible(true);
	m_focusMap->addElement(m_layout);
	m_focusMap->addElement(m_videoMenu);
	m_focusMap->focusElement(m_videoMenu);
	enactCallback((int)OptionsMenu::Interact::on_video);
}

void OptionsMenu::graphics() 
{
	// Remove control from the video menu
	m_videoMenu->setVisible(false);
	m_focusMap->clear();

	// Transfer control to the graphics menu
	m_gfxMenu->setVisible(true);
	m_focusMap->addElement(m_layout);
	m_focusMap->addElement(m_gfxMenu);
	m_focusMap->focusElement(m_gfxMenu);
	enactCallback((int)OptionsMenu::Interact::on_graphics);
}

void OptionsMenu::controls() 
{
	enactCallback((int)OptionsMenu::Interact::on_controls);
}

void OptionsMenu::back() 
{
	m_videoMenu->setVisible(false);
	m_gfxMenu->setVisible(false);
	m_focusMap->clear();
	m_engine.getPreferenceState().save();

	// Revert appearance and control back to previous element (start menu, pause menu, etc)
	m_engine.getModule_UI().popRootElement();
	m_layout->setSelectionIndex(-1);
	enactCallback((int)OptionsMenu::Interact::on_back);
}