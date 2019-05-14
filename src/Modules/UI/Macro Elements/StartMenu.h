#pragma once
#ifndef STARTMENU_H
#define STARTMENU_H

#include "Modules/UI/Basic Elements/UI_Element.h"
#include "Modules/UI/Basic Elements/Button.h"
#include "Modules/UI/Basic Elements/Layout_Vertical.h"
#include "Modules/UI/Basic Elements/Panel.h"
#include "Modules/UI/Macro Elements/OptionsMenu.h"
#include "Engine.h"


/** A UI element serving as a start menu. */
class StartMenu : public UI_Element
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
	inline StartMenu(Engine * engine) : UI_Element() {
		// Make a vertical layout to house list items
		auto layout = std::make_shared<Layout_Vertical>(engine);
		layout->setSpacing(10.0f);
		m_layout = layout;
		addElement(layout);

		// Title
		auto title = std::make_shared<Label>(engine, "Main Menu");
		title->setTextScale(20.0f);
		title->setAlignment(Label::align_center);
		m_title = title;
		addElement(title);

		// Add 'Start' button
		auto startButton = std::make_shared<Button>(engine, "Start");
		startButton->addCallback(UI_Element::on_mouse_release, [&, engine]() { 
			setVisible(false);
			m_optionsMenu->setVisible(false);
			engine->getModule_Game().startGame(); 
			enactCallback(on_start); 
		});
		layout->addElement(startButton);
			
		// Add 'Options' button
		auto optionsButton = std::make_shared<Button>(engine, "       Options      >");
		m_optionsMenu = std::make_shared<OptionsMenu>(engine);
		m_optionsMenu->setVisible(false);
		optionsButton->addCallback(UI_Element::on_mouse_release, [&]() {
			m_layout->setVisible(false);
			m_title->setVisible(false);
			m_optionsMenu->setVisible(true);
			enactCallback(on_options); 
		});
		m_optionsMenu->addCallback(OptionsMenu::on_back, [&]() {
			m_layout->setVisible(true);
			m_title->setVisible(true);
			m_optionsMenu->setVisible(false);
		});
		layout->addElement(optionsButton);
		addElement(m_optionsMenu);
		
		// Add 'Quit' button
		auto quitButton = std::make_shared<Button>(engine, "Quit");
		quitButton->addCallback(UI_Element::on_mouse_release, [&, engine]() {
			setVisible(false);
			m_optionsMenu->setVisible(false);
			engine->shutDown();
			enactCallback(on_quit);
		});
		layout->addElement(quitButton);
	}


	// Public Interface Implementations
	inline virtual void setScale(const glm::vec2 & scale) override {
		UI_Element::setScale(scale);
		m_layout->setScale({ 125, 100 });
		m_layout->setPosition(glm::vec2(250, scale.y - 400));
		m_title->setPosition({ 250, scale.y - 100 });
		m_optionsMenu->setScale(scale);
		enactCallback(on_resize);
	}


private:
	// Private Attributes
	std::shared_ptr<UI_Element> m_layout, m_title, m_optionsMenu;
};

#endif // STARTMENU_H