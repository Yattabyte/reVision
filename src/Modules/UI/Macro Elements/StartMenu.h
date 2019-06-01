#pragma once
#ifndef STARTMENU_H
#define STARTMENU_H

#include "Modules/UI/Macro Elements/Menu.h"
#include "Modules/UI/Macro Elements/OptionsMenu.h"
#include "Engine.h"


/** A UI element serving as a start menu. */
class StartMenu : public Menu {
public:
	// Public Interaction Enums
	const enum interact {
		on_start_game = last_interact_index,
		on_start_puzzle,
		on_options,
		on_quit,
	};


	// Public (de)Constructors
	/** Destroy the start menu. */
	inline ~StartMenu() = default;
	/** Construct a start menu. 
	@param	engine		the engine to use. */
	inline StartMenu(Engine * engine, UI_Element * parent = nullptr)
		: Menu(engine, parent) {
		// Title
		m_title->setText("MAIN MENU");

		// Add 'Start Game' button
		addButton(engine, "START GAME", [&]() { startGame(); });

		// Add 'Start Puzzle' button
		addButton(engine, "START PUZZLE", [&]() { startPuzzle(); });

		// Add 'Options' button
		m_optionsMenu = std::make_shared<OptionsMenu>(engine);
		addButton(engine, "  OPTIONS >", [&]() { options(); });

		// Add 'Quit' button
		addButton(engine, "QUIT", [&]() { quit(); });

		// Callbacks
		addCallback(UI_Element::on_resize, [&]() {
			const auto scale = getScale();
			m_optionsMenu->setScale(scale);
		});
	}


protected:
	// Protected Methods
	/** Choose 'start game' from the main menu. */
	inline void startGame() {
		enactCallback(on_start_game);
		m_engine->getModule_UI().clear();
	}
	/** Choose 'start puzzle' from the main menu. */
	inline void startPuzzle() {
		enactCallback(on_start_puzzle);
	}
	/** Choose 'options' from the main menu. */
	inline void options() {
		// Transfer appearance and control to options menu
		m_engine->getModule_UI().pushRootElement(m_optionsMenu);
		m_layout->setSelectionIndex(-1);
		enactCallback(on_options);
	}
	/** Choose 'quit' from the main menu. */
	inline void quit() {
		m_engine->getModule_UI().clear();
		m_engine->shutDown();
		enactCallback(on_quit);
	}


	// Protected Attributes
	std::shared_ptr<UI_Element> m_optionsMenu;
};

#endif // STARTMENU_H