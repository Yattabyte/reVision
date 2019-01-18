#pragma once
#ifndef OPTIONSMENU_H
#define OPTIONSMENU_H

#include "Modules/UI/Basic Elements/UI_Element.h"
#include "Modules/UI/Basic Elements/Button.h"
#include "Modules/UI/Basic Elements/Toggle.h"
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
		setScale(glm::vec2(250, 400));
		mainLayout->setScale(glm::vec2(250, 400));
		addElement(mainLayout);

		// Title
		auto title = std::make_shared<Label>(engine);
		mainLayout->addElement(title);
		title->setText("Options");
		title->setTextScale(20.0f);
		
		// Window Options
		auto windowPanel = std::make_shared<Panel>(engine);
		auto windowLayout = std::make_shared<Layout_Vertical>(engine);
		auto graphicsPanel = std::make_shared<Panel>(engine);
		auto graphicsLayout = std::make_shared<Layout_Vertical>(engine);
		{
			windowPanel->setScale(glm::vec2(250, 100));
			mainLayout->addElement(windowPanel);
			windowPanel->addElement(windowLayout);

			// Title
			auto windowTitle = std::make_shared<Label>(engine);
			windowLayout->addElement(windowTitle);
			windowTitle->setText("Window Options");
			windowTitle->setTextScale(15.0f);
			windowTitle->setAlignment(Label::Alignment::align_left);

			auto e1 = std::make_shared<Layout_Horizontal>(engine);
			windowLayout->addElement(e1);
			auto e1label = std::make_shared<Label>(engine);
			e1->addElement(e1label);
			e1label->setText("Resolution:");
			e1label->setAlignment(Label::Alignment::align_left);
			auto e1option = std::make_shared<Button>(engine);
			e1->addElement(e1option);

			auto e2 = std::make_shared<Layout_Horizontal>(engine);
			windowLayout->addElement(e2);
			auto e2label = std::make_shared<Label>(engine);
			e2->addElement(e2label);
			e2label->setText("Refresh-rate:");
			e2label->setAlignment(Label::Alignment::align_left);
			auto e2option = std::make_shared<Button>(engine);
			e2->addElement(e2option);

			auto e3 = std::make_shared<Layout_Horizontal>(engine);
			windowLayout->addElement(e3);
			auto e3label = std::make_shared<Label>(engine);
			e3->addElement(e3label);
			e3label->setText("Gamma:");
			e3label->setAlignment(Label::Alignment::align_left);
			auto e3option = std::make_shared<Button>(engine);
			e3->addElement(e3option);

			auto e4 = std::make_shared<Layout_Horizontal>(engine);
			windowLayout->addElement(e4);
			auto e4label = std::make_shared<Label>(engine);
			e4->addElement(e4label);
			e4label->setText("VSync:");
			e4label->setAlignment(Label::Alignment::align_left);
			auto e4option = std::make_shared<Toggle>(engine);
			e4->addElement(e4option);
		}
		// Graphics Options
		{
			graphicsPanel->setScale(glm::vec2(250, 100));
			mainLayout->addElement(graphicsPanel);
			graphicsPanel->addElement(graphicsLayout);

			auto graphicsTitle = std::make_shared<Label>(engine);
			graphicsLayout->addElement(graphicsTitle);
			graphicsTitle->setText("Graphics Options");
			graphicsTitle->setTextScale(15.0f);
			graphicsTitle->setAlignment(Label::Alignment::align_left);

			auto e1 = std::make_shared<Layout_Horizontal>(engine);
			graphicsLayout->addElement(e1);
			auto e1label = std::make_shared<Label>(engine);
			e1->addElement(e1label);
			e1label->setText("Bloom:");
			e1label->setAlignment(Label::Alignment::align_left);
			auto e1option = std::make_shared<Toggle>(engine);
			e1->addElement(e1option);

			auto e2 = std::make_shared<Layout_Horizontal>(engine);
			graphicsLayout->addElement(e2);
			auto e2label = std::make_shared<Label>(engine);
			e2->addElement(e2label);
			e2label->setText("SSAO:");
			e2label->setAlignment(Label::Alignment::align_left);
			auto e2option = std::make_shared<Toggle>(engine);
			e2->addElement(e2option);

			auto e3 = std::make_shared<Layout_Horizontal>(engine);
			graphicsLayout->addElement(e3);
			auto e3label = std::make_shared<Label>(engine);
			e3->addElement(e3label);
			e3label->setText("SSR:");
			e3label->setAlignment(Label::Alignment::align_left);
			auto e3option = std::make_shared<Toggle>(engine);
			e3->addElement(e3option);

			auto e4 = std::make_shared<Layout_Horizontal>(engine);
			graphicsLayout->addElement(e4);
			auto e4label = std::make_shared<Label>(engine);
			e4->addElement(e4label);
			e4label->setText("FXAA:");
			e4label->setAlignment(Label::Alignment::align_left);
			auto e4option = std::make_shared<Toggle>(engine);
			e4->addElement(e4option);
		}
		windowLayout->setMargin(20.0f);
		graphicsLayout->setMargin(20.0f);
		windowLayout->setScale(windowPanel->getScale());
		graphicsLayout->setScale(graphicsPanel->getScale());

		auto backButton = std::make_shared<Button>(engine);
		backButton->setText("Back");
		backButton->setBevelRadius(15.0F);
		backButton->addCallback(UI_Element::on_mouse_release, [&]() {enactCallback(on_back); });
		backButton->setMaxScale(glm::vec2(75, 25));
		mainLayout->addElement(backButton);
	}
};

#endif // OPTIONSMENU_H