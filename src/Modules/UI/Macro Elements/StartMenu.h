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
	inline StartMenu(Engine * engine) : Menu(engine), m_engine(engine) {
		// Title
		m_title->setText("MAIN MENU");

		// Add 'Start Game' button
		auto startButton = std::make_shared<Button>(engine, "START GAME");
		addButton(startButton, [&]() { pressStartGame(); });

		// Add 'Start Puzzle' button
		auto puzzleButton = std::make_shared<Button>(engine, "START PUZZLE");
		addButton(puzzleButton, [&]() { pressStartPuzzle(); });

		// Add 'Options' button
		auto optionsButton = std::make_shared<Button>(engine, "  OPTIONS >");
		m_optionsMenu = std::make_shared<OptionsMenu>(engine);
		addButton(optionsButton, [&]() { pressOptions(); });

		// Add 'Quit' button
		auto quitButton = std::make_shared<Button>(engine, "QUIT");
		addButton(quitButton, [&]() { pressQuit(); });
	}


	// Public Interface Implementations
	inline virtual void setScale(const glm::vec2 & scale) override {
		m_optionsMenu->setScale(scale);
		Menu::setScale(scale);
	}
	inline virtual void userAction(ActionState & actionState) override {
		// Start menu doesn't implement any custom controls, focus is on the list
		m_layout->userAction(actionState);
	}


protected:
	// Protected Methods
	/***/
	inline void pressStartGame() {
		enactCallback(on_start_game);
		m_engine->getModule_UI().clear();
	}
	/***/
	inline void pressStartPuzzle() {
		enactCallback(on_start_puzzle);
	}
	/***/
	inline void pressOptions() {
		// Transfer appearance and control to options menu
		m_engine->getModule_UI().pushRootElement(m_optionsMenu);
		m_layout->setSelectionIndex(-1);
		enactCallback(on_options);
	}
	/***/
	inline void pressQuit() {
		m_engine->getModule_UI().clear();
		m_engine->shutDown();
		enactCallback(on_quit);
	}


	// Protected Attributes
	Engine * m_engine = nullptr;
	std::shared_ptr<UI_Element> m_optionsMenu;
};

#endif // STARTMENU_H