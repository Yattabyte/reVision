#pragma once
#ifndef OPTIONSMENU_H
#define OPTIONSMENU_H

#include "Modules/UI/Basic Elements/UI_Element.h"
#include "Modules/UI/Basic Elements/Button.h"
#include "Modules/UI/Basic Elements/Layout_Horizontal.h"
#include "Modules/UI/Basic Elements/Layout_Vertical.h"
#include "Modules/UI/Basic Elements/Panel.h"
#include "Engine.h"


/** A UI element serving as an options menu. */
class OptionsMenu : public Panel
{
public:
	// UI interaction enums
	enum interact {
		on_back = last_interact_index
	};

	// (de)Constructors
	~OptionsMenu() = default;
	OptionsMenu(Engine * engine) : Panel(engine) {
		auto mainLayout = std::make_shared<Layout_Vertical>(engine);
		setScale(glm::vec2(250, 250));
		mainLayout->setScale(glm::vec2(250, 250));
		addElement(mainLayout);

		auto backButton = std::make_shared<Button>(engine);
		backButton->setText("Back");
		backButton->setBevelRadius(15.0F);
		backButton->addCallback(UI_Element::on_mouse_release, [&]() {enactCallback(on_back); });
		mainLayout->addElement(backButton);
	}
};

#endif // OPTIONSMENU_H