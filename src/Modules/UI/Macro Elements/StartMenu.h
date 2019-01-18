#pragma once
#ifndef STARTMENU_H
#define STARTMENU_H

#include "Modules/UI/Basic Elements/UI_Element.h"
#include "Modules/UI/Basic Elements/Button.h"
#include "Modules/UI/Basic Elements/Layout_Vertical.h"
#include "Modules/UI/Basic Elements/Panel.h"
#include "Engine.h"


/** A UI element serving as a start menu. */
class StartMenu : public Panel
{
public:
	// UI interaction enums
	enum interact {
		on_start = last_interact_index,
		on_options,
		on_controls,
		on_quit,
	};

	// (de)Constructors
	~StartMenu() = default;
	StartMenu(Engine * engine) : Panel(engine) {
		auto mainLayout = std::make_shared<Layout_Vertical>(engine);
		setScale(glm::vec2(125, 250));
		mainLayout->setScale(glm::vec2(125, 250));
		mainLayout->setSpacing(5.0f);
		addElement(mainLayout);

		// Title
		auto title = std::make_shared<Label>(engine);
		mainLayout->addElement(title);
		title->setText("Main Menu");
		title->setTextScale(20.0f);

		// Add 'Start' button
		auto startButton = std::make_shared<Button>(engine);
		startButton->setText("Start");
		startButton->setBevelRadius(10.0F);
		startButton->addCallback(UI_Element::on_mouse_release, [&]() {enactCallback(on_start); });
		mainLayout->addElement(startButton);
			
		// Add 'Options' button
		auto optionsButton = std::make_shared<Button>(engine);
		optionsButton->setText("Options");
		optionsButton->setBevelRadius(10.0F);
		optionsButton->setMaxScale(glm::vec2(75, 25));
		optionsButton->addCallback(UI_Element::on_mouse_release, [&]() {enactCallback(on_options); });
		mainLayout->addElement(optionsButton);

		// Add 'Controls' button
		auto controlsButton = std::make_shared<Button>(engine);
		controlsButton->setText("Controls (disabled)");
		controlsButton->setBevelRadius(10.0F);
		controlsButton->setEnabled(false);
		controlsButton->setMinScale(glm::vec2(75, 75));
		controlsButton->addCallback(UI_Element::on_mouse_release, [&]() {enactCallback(on_controls); });
		mainLayout->addElement(controlsButton);

		// Add 'Quit' button
		auto quitButton = std::make_shared<Button>(engine);
		quitButton->setText("Quit");
		quitButton->setBevelRadius(10.0F);
		quitButton->addCallback(UI_Element::on_mouse_release, [&]() {enactCallback(on_quit); });
		mainLayout->addElement(quitButton);
	}
};

#endif // STARTMENU_H