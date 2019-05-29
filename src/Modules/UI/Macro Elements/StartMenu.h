#pragma once
#ifndef STARTMENU_H
#define STARTMENU_H

#include "Modules/UI/Macro Elements/Menu.h"
#include "Modules/UI/Macro Elements/OptionsMenu.h"
#include "Modules/UI/Basic Elements/Button.h"
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
		startButton->addCallback(Button::on_clicked, [&]() { pressStartGame(); });
		addButton(startButton);

		// Add 'Start Puzzle' button
		auto puzzleButton = std::make_shared<Button>(engine, "START PUZZLE");
		puzzleButton->addCallback(Button::on_clicked, [&]() { pressStartPuzzle(); });
		addButton(puzzleButton);

		// Add 'Options' button
		auto optionsButton = std::make_shared<Button>(engine, "  OPTIONS >");
		m_optionsMenu = std::make_shared<OptionsMenu>(engine);
		optionsButton->addCallback(Button::on_clicked, [&]() { pressOptions(); });		
		addButton(optionsButton);

		// Add 'Quit' button
		auto quitButton = std::make_shared<Button>(engine, "QUIT");
		quitButton->addCallback(Button::on_clicked, [&, engine]() { pressQuit(); });
		addButton(quitButton);
		m_layout->setIndex(0);
	}


	// Public Interface Implementations
	inline virtual void setScale(const glm::vec2 & scale) override {
		m_optionsMenu->setScale(scale);
		Menu::setScale(scale);
	}
	inline virtual bool userAction(ActionState & actionState) override {
		if (actionState.isAction(ActionState::UI_UP) == ActionState::PRESS) {
			m_layout->setIndex(m_layout->getIndex() - 1);
			return true;
		}
		else if (actionState.isAction(ActionState::UI_DOWN) == ActionState::PRESS) {
			m_layout->setIndex(m_layout->getIndex() + 1);
			return true;
		}
		else if (actionState.isAction(ActionState::UI_ENTER) == ActionState::PRESS) {
			if (auto index = m_layout->getIndex(); index > -1)
				m_layout->getElement(index)->fullPress();
			return true;
		}
		return false;
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
		m_engine->getModule_UI().pushRootElement(m_optionsMenu);
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