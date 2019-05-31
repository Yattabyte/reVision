#pragma once
#ifndef PAUSEMENU_H
#define PAUSEMENU_H

#include "Modules/UI/Macro Elements/Menu.h"
#include "Modules/UI/Macro Elements/OptionsMenu.h"
#include "Modules/UI/Basic Elements/Button.h"
#include "Engine.h"


/** A UI element serving as a pause menu. */
class PauseMenu : public Menu {
public:
	// Public Interaction Enums
	const enum interact {
		on_resume_game = last_interact_index,
		on_options,
		on_quit,
	};


	// Public (de)Constructors
	/** Destroy the start menu. */
	inline ~PauseMenu() = default;
	/** Construct a start menu. 
	@param	engine		the engine to use. */
	inline PauseMenu(Engine * engine) : Menu(engine) {
		// Title
		m_title->setText("PAUSE MENU");

		// Add 'Start Game' button
		auto startButton = std::make_shared<Button>(engine, "RESUME");
		addButton(startButton, [&]() { pressResume(); });
			
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
	inline void pressResume() {
		enactCallback(on_resume_game);
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
	std::shared_ptr<UI_Element> m_optionsMenu;
};

#endif // PAUSEMENU_H