#pragma once
#ifndef STARTMENU_H
#define STARTMENU_H

#include "Modules/UI/Basic Elements/UI_Element.h"
#include "Modules/UI/Basic Elements/Button.h"
#include "Modules/UI/Basic Elements/Layout_Horizontal.h"
#include "Modules/UI/Basic Elements/Layout_Vertical.h"
#include "Modules/UI/Basic Elements/Panel.h"
#include "Engine.h"


/** A layout class which controls the position and sizes of its children, laying them out evenly in a row. */
class StartMenu : public Panel
{
public:
	// UI interaction enums
	enum interact {
		on_start = UI_Element::on_mouse_release + 1,
		on_quit,
	};

	// (de)Constructors
	~StartMenu() = default;
	StartMenu(Engine * engine) : Panel(engine) {
		auto mainLayout = std::make_shared<Layout_Vertical>(engine);
		setScale(glm::vec2(250, 350));
		mainLayout->setScale(glm::vec2(250, 350));
		addElement(mainLayout);

		// Add 'Start' button
		auto startButton = std::make_shared<Button>(engine);
		startButton->setText("Start");
		startButton->setBevelRadius(15.0F);
		startButton->addCallback(UI_Element::on_mouse_release, [&]() {enactCallback(on_start); });		
		mainLayout->addElement(startButton);

		// Add 'Options' button
		auto optionsButton = std::make_shared<Button>(engine);
		optionsButton->setText("Options (disabled)");
		optionsButton->setBevelRadius(15.0F);
		optionsButton->setEnabled(false);
		mainLayout->addElement(optionsButton);

		// Add 'Controls' button
		auto controlsButton = std::make_shared<Button>(engine);
		controlsButton->setText("Controls (disabled)");
		controlsButton->setBevelRadius(15.0F);
		controlsButton->setEnabled(false);
		mainLayout->addElement(controlsButton);

		// Add 'Quit' button
		auto quitButton = std::make_shared<Button>(engine);
		quitButton->setText("Quit");
		quitButton->setBevelRadius(15.0F);
		quitButton->addCallback(UI_Element::on_mouse_release, [&]() {enactCallback(on_quit); });
		mainLayout->addElement(quitButton);

		// Horizontal Layout
		auto label = std::make_shared<Label>(engine);
		label->setText("Horizontal Layout:");
		label->setTextScale(20.0f);
		label->setAlignment(Label::Alignment::align_left);
		mainLayout->addElement(label);
		auto panel = std::make_shared<Panel>(engine);
		mainLayout->addElement(panel);
		auto testLayout = std::make_shared<Layout_Horizontal>(engine);
		testLayout->setScale(panel->getScale());
		testLayout->setSpacing(2.0f);
		testLayout->addElement(std::make_shared<Button>(engine));
		auto testLayout2 = std::make_shared<Layout_Vertical>(engine);
		testLayout2->setMargin(0.0f);
		testLayout2->setSpacing(2.0f);
		testLayout2->addElement(std::make_shared<Button>(engine));
		testLayout2->addElement(std::make_shared<Button>(engine));
		testLayout->addElement(testLayout2);
		auto testLayout3 = std::make_shared<Layout_Horizontal>(engine);
		testLayout->addElement(testLayout3);
		testLayout3->addElement(std::make_shared<Button>(engine));
		testLayout3->addElement(std::make_shared<Button>(engine));
		testLayout3->setMargin(0.0f);
		testLayout3->setSpacing(2.0f);
		testLayout->addElement(std::make_shared<Button>(engine));
		panel->addElement(testLayout);
	}
};

#endif // STARTMENU_H