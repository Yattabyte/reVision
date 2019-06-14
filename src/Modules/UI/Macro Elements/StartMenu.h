#pragma once
#ifndef STARTMENU_H
#define STARTMENU_H

#include "Modules/UI/Macro Elements/Menu.h"
#include "Modules/UI/Macro Elements/OptionsMenu.h"
#include "Modules/UI/FocusMap.h"
#include "Engine.h"


/** A UI element serving as a start menu. */
class StartMenu : public Menu {
public:
	// Public Interaction Enums
	const enum interact {
		on_start_game = last_interact_index,
		on_options,
		on_quit,
	};


	// Public (de)Constructors
	/** Destroy the start menu. */
	inline ~StartMenu() = default;
	/** Construct a start menu. 
	@param	engine		the engine to use. */
	inline StartMenu(Engine * engine)
		: Menu(engine) {
		// Title
		m_title->setText("MAIN MENU");

		// Add 'Start Game' button
		addButton(engine, "START GAME", [&]() { startGame(); });

		// Add 'Options' button
		m_optionsMenu = std::make_shared<OptionsMenu>(engine);
		addButton(engine, "  OPTIONS >", [&]() { goToOptions(); });
		m_optionsMenu->addCallback(OptionsMenu::on_back, [&]() { returnFromOptions(); });

		// Add 'Quit' button
		addButton(engine, "QUIT", [&]() { quit(); });

		// Callbacks
		addCallback(UI_Element::on_resize, [&]() {
			const auto scale = getScale();
			m_optionsMenu->setScale(scale);
		});
		
		// Populate Focus Map
		m_focusMap = std::make_shared<FocusMap>();
		m_focusMap->addElement(m_layout);
		m_focusMap->focusElement(m_layout);
		m_engine->getModule_UI().setFocusMap(getFocusMap());
	}


protected:
	// Protected Methods
	/** Choose 'start game' from the main menu. */
	inline void startGame() {
		m_engine->getModule_UI().clear();
		enactCallback(on_start_game);
	}
	/** Choose 'options' from the main menu. */
	inline void goToOptions() {
		// Transfer appearance and control to options menu
		auto & ui = m_engine->getModule_UI();
		ui.pushRootElement(m_optionsMenu);
		ui.setFocusMap(m_optionsMenu->getFocusMap());
		m_layout->setSelectionIndex(-1);
		enactCallback(on_options);
	}
	/** Chosen when control is returned from the options menu. */
	inline void returnFromOptions() {
		// Transfer control back to this menu
		m_engine->getModule_UI().setFocusMap(getFocusMap());
	}
	/** Choose 'quit' from the main menu. */
	inline void quit() {
		m_engine->getModule_UI().clear();
		m_engine->shutDown();
		enactCallback(on_quit);
	}


	// Protected Attributes
	std::shared_ptr<OptionsMenu> m_optionsMenu;
};

#endif // STARTMENU_H