#pragma once
#ifndef PAUSEMENU_H
#define PAUSEMENU_H

#include "Modules/UI/Macro Elements/Menu.h"
#include "Modules/UI/Macro Elements/OptionsMenu.h"
#include "Engine.h"


/** A UI element serving as a pause menu. */
class PauseMenu : public Menu {
public:
	// Public Interaction Enums
	enum interact {
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
		startButton->addCallback(Button::on_clicked, [&]() {
			setVisible(false);
			enactCallback(on_resume_game); 
		});
		addButton(startButton);
			
		// Add 'Options' button
		auto optionsButton = std::make_shared<Button>(engine, "  OPTIONS >");
		m_optionsMenu = std::make_shared<OptionsMenu>(engine);
		m_optionsMenu->setVisible(false);
		optionsButton->addCallback(Button::on_clicked, [&]() {
			m_backPanel->setVisible(false);
			m_optionsMenu->setVisible(true);
			enactCallback(on_options); 
		});
		m_optionsMenu->addCallback(OptionsMenu::on_back, [&]() {
			m_backPanel->setVisible(true);
			m_optionsMenu->setVisible(false);
		});
		addButton(optionsButton);
		addElement(m_optionsMenu);
		
		// Add 'Quit' button
		auto quitButton = std::make_shared<Button>(engine, "QUIT");
		quitButton->addCallback(Button::on_clicked, [&, engine]() {
			setVisible(false);
			engine->shutDown();
			enactCallback(on_quit);
		});
		addButton(quitButton);
	}


	// Public Interface Implementations
	inline virtual void setScale(const glm::vec2 & scale) override {
		m_optionsMenu->setScale(scale);
		Menu::setScale(scale);
	}


protected:
	// Protected Attributes
	std::shared_ptr<UI_Element> m_optionsMenu;
};

#endif // PAUSEMENU_H