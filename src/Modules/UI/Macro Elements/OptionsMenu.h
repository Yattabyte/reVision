#pragma once
#ifndef OPTIONSMENU_H
#define OPTIONSMENU_H

#include "Modules/UI/Basic Elements/UI_Element.h"
#include "Modules/UI/Basic Elements/Button.h"
#include "Modules/UI/Basic Elements/List.h"
#include "Modules/UI/Basic Elements/Toggle.h"
#include "Modules/UI/Basic Elements/Layout_Horizontal.h"
#include "Modules/UI/Basic Elements/Layout_Vertical.h"
#include "Modules/UI/Basic Elements/Panel.h"
#include "Modules/UI/Decorators/Border.h"
#include "Modules/UI/Decorators/Scrollbar_V.h"
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
		{
			auto windowLayout = std::make_shared<Layout_Vertical>(engine);
			windowLayout->setMargin(5.0f);
			windowLayout->setSpacing(1.0f);
			auto windowBorder = std::make_shared<Border>(engine, windowLayout);
			mainLayout->addElement(windowBorder);

			// Title
			auto windowTitle = std::make_shared<Label>(engine, "Window Options");
			windowLayout->addElement(windowTitle);
			windowTitle->setTextScale(15.0f);

			auto e1 = std::make_shared<Layout_Horizontal>(engine);
			windowLayout->addElement(e1);
			e1->addElement(std::make_shared<Label>(engine, "Resolution:"));
			auto e1Drop = std::make_shared<List>(engine);
			m_resolutions = engine->getResolutions();
			for each (const auto & res in m_resolutions) {
				const auto string  = std::to_string(res.x) + "x" + std::to_string(res.y);
				auto & label = std::make_shared<Label>(engine, string);
				label->setColor(glm::vec3(0.0f));
				label->setAlignment(Label::align_center);
				e1Drop->addListElement(label);
			}
			e1Drop->addCallback(List::on_index_changed, [&, e1Drop, engine]() {
				const auto & index = e1Drop->getIndex();
				engine->getPreferenceState().setValue(PreferenceState::C_WINDOW_WIDTH, m_resolutions[index].x);
				engine->getPreferenceState().setValue(PreferenceState::C_WINDOW_HEIGHT, m_resolutions[index].y);
			});
			e1->addElement(e1Drop);
			
			auto e2 = std::make_shared<Layout_Horizontal>(engine);
			e2->setMaxScale(glm::vec2(e2->getMaxScale().x, 25.0f));
			windowLayout->addElement(e2);
			e2->addElement(std::make_shared<Label>(engine, "Refresh-rate:"));
			auto e2option = std::make_shared<Button>(engine);
			e2->addElement(e2option);

			auto e3 = std::make_shared<Layout_Horizontal>(engine);
			e3->setMaxScale(glm::vec2(e3->getMaxScale().x, 25.0f));
			windowLayout->addElement(e3);
			e3->addElement(std::make_shared<Label>(engine, "Gamma:"));
			auto e3option = std::make_shared<Button>(engine);
			e3->addElement(e3option);

			auto e4 = std::make_shared<Layout_Horizontal>(engine);
			e4->setMaxScale(glm::vec2(e4->getMaxScale().x, 25.0f));
			windowLayout->addElement(e4);
			e4->addElement(std::make_shared<Label>(engine, "VSync:"));
			auto e4option = std::make_shared<Toggle>(engine);
			e4->addElement(e4option);

			auto e5 = std::make_shared<Layout_Horizontal>(engine);
			e5->setMaxScale(glm::vec2(e5->getMaxScale().x, 25.0f));
			windowLayout->addElement(e5);
			e5->addElement(std::make_shared<Label>(engine, "Full-screen:"));
			bool e5State = true;
			engine->getPreferenceState().getOrSetValue<bool>(PreferenceState::C_WINDOW_FULLSCREEN, e5State);
			auto e5option = std::make_shared<Toggle>(engine, e5State);
			e5option->addCallback(Toggle::on_toggle, [&, e5option, engine]() {
				engine->getPreferenceState().setValue(PreferenceState::C_WINDOW_FULLSCREEN, e5option->getToggled() ? 1.0f : 0.0f);
			});
			e5->addElement(e5option);
		}
		// Graphics Options
		{
			auto graphicsLayout = std::make_shared<Layout_Vertical>(engine);
			graphicsLayout->setMargin(5.0f);
			graphicsLayout->setSpacing(1.0f);
			auto graphicsBorder = std::make_shared<Border>(engine, graphicsLayout);
			graphicsBorder->setMaxScale(glm::vec2(graphicsBorder->getMaxScale().x, 150.0f));
			mainLayout->addElement(graphicsBorder);

			auto graphicsTitle = std::make_shared<Label>(engine, "Graphics Options");
			graphicsLayout->addElement(graphicsTitle);
			graphicsTitle->setTextScale(15.0f);

			auto e1 = std::make_shared<Layout_Horizontal>(engine);
			graphicsLayout->addElement(e1);
			e1->addElement(std::make_shared<Label>(engine, "Bloom:"));
			bool e1State = true;
			engine->getPreferenceState().getOrSetValue<bool>(PreferenceState::C_BLOOM, e1State);
			auto e1option = std::make_shared<Toggle>(engine, e1State);
			e1option->addCallback(Toggle::on_toggle, [&, e1option, engine]() {
				engine->getPreferenceState().setValue(PreferenceState::C_BLOOM, e1option->getToggled() ? 1.0f : 0.0f); 
			});
			e1->addElement(e1option);

			auto e2 = std::make_shared<Layout_Horizontal>(engine);
			graphicsLayout->addElement(e2);
			e2->addElement(std::make_shared<Label>(engine, "SSAO:"));
			bool e2State = true;
			engine->getPreferenceState().getOrSetValue<bool>(PreferenceState::C_SSAO, e2State);
			auto e2option = std::make_shared<Toggle>(engine, e2State);
			e2option->addCallback(Toggle::on_toggle, [&, e2option, engine]() {
				engine->getPreferenceState().setValue(PreferenceState::C_SSAO, e2option->getToggled() ? 1.0f : 0.0f); 
			});
			e2->addElement(e2option);

			auto e3 = std::make_shared<Layout_Horizontal>(engine);
			graphicsLayout->addElement(e3);
			e3->addElement(std::make_shared<Label>(engine, "SSR:"));
			bool e3State = true;
			engine->getPreferenceState().getOrSetValue<bool>(PreferenceState::C_SSR, e3State);
			auto e3option = std::make_shared<Toggle>(engine, e3State);
			e3option->addCallback(Toggle::on_toggle, [&, e3option, engine]() {
				engine->getPreferenceState().setValue(PreferenceState::C_SSR, e3option->getToggled() ? 1.0f : 0.0f); 
			});
			e3->addElement(e3option);

			auto e4 = std::make_shared<Layout_Horizontal>(engine);
			graphicsLayout->addElement(e4);
			e4->addElement(std::make_shared<Label>(engine, "FXAA:"));
			bool e4State = true;
			engine->getPreferenceState().getOrSetValue<bool>(PreferenceState::C_FXAA, e4State);
			auto e4option = std::make_shared<Toggle>(engine, e4State);
			e4option->addCallback(Toggle::on_toggle, [&, e4option, engine]() {
				engine->getPreferenceState().setValue(PreferenceState::C_FXAA, e4option->getToggled() ? 1.0f : 0.0f); 
			});
			e4->addElement(e4option);
		}

		auto bottomBar = std::make_shared<Layout_Horizontal>(engine);
		bottomBar->setMaxScale(glm::vec2(300, 25));
		mainLayout->addElement(bottomBar);

		auto backButton = std::make_shared<Button>(engine, "Back");
		backButton->setBevelRadius(15.0F);
		backButton->addCallback(UI_Element::on_mouse_release, [&]() {
			enactCallback(on_back); 
		});
		bottomBar->addElement(backButton);

		auto saveButton = std::make_shared<Button>(engine, "Save");
		saveButton->setBevelRadius(15.0F);
		saveButton->addCallback(Button::on_mouse_release, [&, engine]() {
			engine->getPreferenceState().save();
		});
		bottomBar->addElement(saveButton);
	}


private:
	// Private Attributes
	std::vector<glm::ivec2> m_resolutions;
};

#endif // OPTIONSMENU_H