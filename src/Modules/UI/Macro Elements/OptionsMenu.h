#pragma once
#ifndef OPTIONSMENU_H
#define OPTIONSMENU_H

#include "Modules/UI/Basic Elements/UI_Element.h"
#include "Modules/UI/Basic Elements/Button.h"
#include "Modules/UI/Basic Elements/DropList.h"
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
		setScale(glm::vec2(250, 500));
		mainLayout->setScale(glm::vec2(250, 500));
		addElement(mainLayout);

		// Title
		auto title = std::make_shared<Label>(engine, "Options");
		mainLayout->addElement(title);
		title->setTextScale(20.0f);
		
		// Window Options
		auto windowPanel = std::make_shared<Panel>(engine);
		auto windowLayout = std::make_shared<Layout_Vertical>(engine);
		auto graphicsPanel = std::make_shared<Panel>(engine);
		auto graphicsLayout = std::make_shared<Layout_Vertical>(engine);
		{
			mainLayout->addElement(windowPanel);
			windowPanel->addElement(windowLayout);

			// Title
			auto windowTitle = std::make_shared<Label>(engine, "Window Options");
			windowLayout->addElement(windowTitle);
			windowTitle->setTextScale(15.0f);

			auto e1 = std::make_shared<Layout_Horizontal>(engine);
			windowLayout->addElement(e1);
			e1->addElement(std::make_shared<Label>(engine, "Resolution:"));
			auto e1option = std::make_shared<DropList>(engine);
			e1->addElement(e1option);

			auto e2 = std::make_shared<Layout_Horizontal>(engine);
			windowLayout->addElement(e2);
			e2->addElement(std::make_shared<Label>(engine, "Refresh-rate:"));
			auto e2option = std::make_shared<Button>(engine);
			e2->addElement(e2option);

			auto e3 = std::make_shared<Layout_Horizontal>(engine);
			windowLayout->addElement(e3);
			e3->addElement(std::make_shared<Label>(engine, "Gamma:"));
			auto e3option = std::make_shared<Button>(engine);
			e3->addElement(e3option);

			auto e4 = std::make_shared<Layout_Horizontal>(engine);
			windowLayout->addElement(e4);
			e4->addElement(std::make_shared<Label>(engine, "VSync:"));
			auto e4option = std::make_shared<Toggle>(engine);
			e4->addElement(e4option);

			auto e5 = std::make_shared<Layout_Horizontal>(engine);
			windowLayout->addElement(e5);
			e5->addElement(std::make_shared<Label>(engine, "Full-screen:"));
			auto e5option = std::make_shared<Toggle>(engine);
			e5->addElement(e5option);
		}
		// Graphics Options
		{
			mainLayout->addElement(graphicsPanel);
			graphicsPanel->addElement(graphicsLayout);

			auto graphicsTitle = std::make_shared<Label>(engine, "Graphics Options");
			graphicsLayout->addElement(graphicsTitle);
			graphicsTitle->setTextScale(15.0f);

			auto e1 = std::make_shared<Layout_Horizontal>(engine);
			graphicsLayout->addElement(e1);
			e1->addElement(std::make_shared<Label>(engine, "Bloom:"));
			auto e1option = std::make_shared<Toggle>(engine);
			e1->addElement(e1option);

			auto e2 = std::make_shared<Layout_Horizontal>(engine);
			graphicsLayout->addElement(e2);
			e2->addElement(std::make_shared<Label>(engine, "SSAO:"));
			auto e2option = std::make_shared<Toggle>(engine);
			e2->addElement(e2option);

			auto e3 = std::make_shared<Layout_Horizontal>(engine);
			graphicsLayout->addElement(e3);
			e3->addElement(std::make_shared<Label>(engine, "SSR:"));
			auto e3option = std::make_shared<Toggle>(engine);
			e3->addElement(e3option);

			auto e4 = std::make_shared<Layout_Horizontal>(engine);
			graphicsLayout->addElement(e4);
			e4->addElement(std::make_shared<Label>(engine, "FXAA:"));
			auto e4option = std::make_shared<Toggle>(engine);
			e4->addElement(e4option);
		}
		windowLayout->setMargin(20.0f);
		graphicsLayout->setMargin(20.0f);
		windowLayout->setScale(windowPanel->getScale());
		graphicsLayout->setScale(graphicsPanel->getScale());

		auto bottomBar = std::make_shared<Layout_Horizontal>(engine);
		bottomBar->setMaxScale(glm::vec2(300, 25));
		mainLayout->addElement(bottomBar);

		auto backButton = std::make_shared<Button>(engine, "Back");
		backButton->setBevelRadius(15.0F);
		backButton->addCallback(UI_Element::on_mouse_release, [&]() {enactCallback(on_back); });
		bottomBar->addElement(backButton);

		auto saveButton = std::make_shared<Button>(engine, "Save");
		saveButton->setBevelRadius(15.0F);
		bottomBar->addElement(saveButton);
	}
};

#endif // OPTIONSMENU_H