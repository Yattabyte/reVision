#pragma once
#ifndef STARTMENU_H
#define STARTMENU_H

#include "Modules/UI/Macro Elements/Menu.h"
#include "Modules/UI/Macro Elements/OptionsMenu.h"
#include "Engine.h"


/** A UI element serving as a start menu. */
class StartMenu : public Menu
{
public:
	// UI interaction enums
	enum interact {
		on_start = last_interact_index,
		on_options,
		on_quit,
	};


	// (de)Constructors
	inline ~StartMenu() = default;
	inline StartMenu(Engine * engine) : Menu(engine) {
		// Title
		m_title->setText("MAIN MENU");

		// Add 'Start' button
		auto startButton = std::make_shared<Button>(engine, "START");
		startButton->addCallback(UI_Element::on_mouse_release, [&, engine]() { 
			setVisible(false);
			engine->getModule_Game().startGame(); 
			enactCallback(on_start); 
		});
		addButton(startButton);
			
		// Add 'Options' button
		auto optionsButton = std::make_shared<Button>(engine, "  OPTIONS >");
		m_optionsMenu = std::make_shared<OptionsMenu>(engine);
		m_optionsMenu->setVisible(false);
		optionsButton->addCallback(UI_Element::on_mouse_release, [&]() {
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
		quitButton->addCallback(UI_Element::on_mouse_release, [&, engine]() {
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


private:
	// Private Attributes
	std::shared_ptr<UI_Element> m_optionsMenu;
};

#endif // STARTMENU_H