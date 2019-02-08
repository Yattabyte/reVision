#pragma once
#ifndef OPTIONSMENU_H
#define OPTIONSMENU_H

#include "Modules/UI/Basic Elements/UI_Element.h"
#include "Modules/UI/Basic Elements/Button.h"
#include "Modules/UI/Basic Elements/DropList.h"
#include "Modules/UI/Basic Elements/Label.h"
#include "Modules/UI/Basic Elements/Layout_Horizontal.h"
#include "Modules/UI/Basic Elements/Layout_Vertical.h"
#include "Modules/UI/Basic Elements/List.h"
#include "Modules/UI/Basic Elements/Panel.h"
#include "Modules/UI/Basic Elements/Slider.h"
#include "Modules/UI/Basic Elements/TextInput.h"
#include "Modules/UI/Basic Elements/Toggle.h"
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
		auto TopDownLayout = std::make_shared<Layout_Vertical>(engine);
		setScale(glm::vec2(600, 400));
		TopDownLayout->setScale(glm::vec2(600, 400));
		addElement(TopDownLayout);

		// Title
		auto title = std::make_shared<Label>(engine, "Options");
		TopDownLayout->addElement(title);
		title->setTextScale(20.0f);

		auto mainLayout = std::make_shared<Layout_Horizontal>(engine);
		TopDownLayout->addElement(mainLayout);

		const auto addLabledSetting = [engine](auto & layout, auto & element, const std::string & text) {
			auto horizontalLayout = std::make_shared<Layout_Horizontal>(engine);
			horizontalLayout->addElement(std::make_shared<Label>(engine, text));			
			horizontalLayout->addElement(element);
			layout->addElement(horizontalLayout);
		};

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

			auto element_res = std::make_shared<DropList>(engine);
			float width = 1920.0f, height = 1080.0f;
			engine->getPreferenceState().getOrSetValue(PreferenceState::C_WINDOW_WIDTH, width);
			engine->getPreferenceState().getOrSetValue(PreferenceState::C_WINDOW_HEIGHT, height);
			m_resolutions = engine->getResolutions();
			std::vector<std::string> strings(m_resolutions.size());
			int counter = 0, index = 0;
			for each (const auto & res in m_resolutions) {
				const auto string = std::to_string(res.x) + "x" + std::to_string(res.y) + " @ " + std::to_string(res.z) + "Hz";
				strings[counter] = string;
				if (res.x == width && res.y == height)
					index = counter;
				counter++;
			}
			element_res->setStrings(strings);
			element_res->setIndex(index);
			element_res->addCallback(List::on_index_changed, [&, element_res, engine]() {
				const auto & index = element_res->getIndex();
				engine->getPreferenceState().setValue(PreferenceState::C_WINDOW_WIDTH, m_resolutions[index].x);
				engine->getPreferenceState().setValue(PreferenceState::C_WINDOW_HEIGHT, m_resolutions[index].y);
				engine->getPreferenceState().setValue(PreferenceState::C_WINDOW_REFRESH_RATE, m_resolutions[index].z);
			});
			addLabledSetting(windowLayout, element_res, "Resolution:");

			auto gamma_layout = std::make_shared<Layout_Horizontal>(engine);
			auto gamma_slider = std::make_shared<Slider>(engine);
			auto gamma_tinput = std::make_shared<TextInput>(engine);
			float gamma = 1.0f;
			engine->getPreferenceState().getOrSetValue(PreferenceState::C_GAMMA, gamma);
			gamma_layout->setMargin(0);
			gamma_layout->setMaxScale(glm::vec2(gamma_layout->getMaxScale().x, 12.5));
			gamma_slider->setMaxScale(glm::vec2(gamma_slider->getMaxScale().x, 12.5));
			gamma_tinput->setMaxScale(glm::vec2(25.0f, 12.5));
			gamma_tinput->setText(std::to_string(gamma));
			gamma_slider->setPercentage(gamma / 2.0f);
			gamma_slider->addCallback(Slider::on_slider_change, [&, gamma_slider, gamma_tinput, engine]() {
				const float value = 2.0f * gamma_slider->getPercentage();
				engine->getPreferenceState().setValue(PreferenceState::C_GAMMA, value);
				gamma_tinput->setText(std::to_string(value));
			});
			gamma_tinput->addCallback(TextInput::on_text_change, [&, gamma_slider, gamma_tinput, engine]() {
				float 
					value = 1.0f;
				try 
					{value = std::stof(gamma_tinput->getText());}
				catch (const std::exception& e) 
					{value = 0.0f;}
				engine->getPreferenceState().setValue(PreferenceState::C_GAMMA, value);
				gamma_slider->setPercentage(value / 2.0f);
			});
			gamma_layout->addElement(gamma_slider);
			gamma_layout->addElement(gamma_tinput);
			addLabledSetting(windowLayout, gamma_layout, "Gamma:");
			
			bool element_sync_state = true;
			engine->getPreferenceState().getOrSetValue<bool>(PreferenceState::C_VSYNC, element_sync_state);
			auto element_sync = std::make_shared<Toggle>(engine, element_sync_state);
			element_sync->addCallback(Toggle::on_toggle, [&, element_sync, engine]() {
				engine->getPreferenceState().setValue(PreferenceState::C_VSYNC, element_sync->getToggled() ? 1.0f : 0.0f);
			});
			addLabledSetting(windowLayout, element_sync, "VSync:");

			bool element_fs_state = true;
			engine->getPreferenceState().getOrSetValue<bool>(PreferenceState::C_WINDOW_FULLSCREEN, element_fs_state);
			auto element_fs = std::make_shared<Toggle>(engine, element_fs_state);
			element_fs->addCallback(Toggle::on_toggle, [&, element_fs, engine]() {
				engine->getPreferenceState().setValue(PreferenceState::C_WINDOW_FULLSCREEN, element_fs->getToggled() ? 1.0f : 0.0f);
			});
			addLabledSetting(windowLayout, element_fs, "Full-screen:");
		}
		// Graphics Options
		{
			auto graphicsLayout = std::make_shared<Layout_Vertical>(engine);
			graphicsLayout->setMargin(5.0f);
			graphicsLayout->setSpacing(1.0f);
			auto graphicsBorder = std::make_shared<Border>(engine, graphicsLayout);
			mainLayout->addElement(graphicsBorder);

			auto graphicsTitle = std::make_shared<Label>(engine, "Graphics Options");
			graphicsLayout->addElement(graphicsTitle);
			graphicsTitle->setTextScale(15.0f);

			
			float materialSize = 1024;
			engine->getPreferenceState().getOrSetValue(PreferenceState::C_MATERIAL_SIZE, materialSize);
			auto element_material_list = std::make_shared<DropList>(engine);
			const std::vector<std::string> strings =	{ "Low",	"Medium",	"High",		"Very High",	"Ultra" };
			const std::vector<float> sizes =			{ 128.0f,	256.0f,		512.0f,		1024.0f,		2048.0f };
			int counter = 0, index = 0;
			for each (const auto & size in sizes) {
				if (materialSize == size)
					index = counter;
				counter++;
			}
			element_material_list->setStrings(strings);
			element_material_list->setIndex(index);
			element_material_list->addCallback(List::on_index_changed, [&, sizes, element_material_list, engine]() {
				engine->getPreferenceState().setValue(PreferenceState::C_MATERIAL_SIZE, sizes[element_material_list->getIndex()]);
			});
			addLabledSetting(graphicsLayout, element_material_list, "Texture Quality:");

			float shadowSize = 1024, shadowQuality = 4;
			engine->getPreferenceState().getOrSetValue(PreferenceState::C_SHADOW_SIZE_SPOT, shadowSize);
			engine->getPreferenceState().getOrSetValue(PreferenceState::C_SHADOW_QUALITY, shadowQuality);
			auto element_shadow_list = std::make_shared<DropList>(engine);
			const std::vector<std::string> strings2 =	{ "Low",	"Medium",	"High",		"Very High",	"Ultra" };
			const std::vector<float> sizes2 =			{ 128.0f,	256.0f,		512.0f,		1024.0f,		2048.0f };
			const std::vector<float> qualities =		{ 1,		2,			3,			4,				5 };
			counter = 0;
			index = 0;
			for each (const auto & size in sizes2) {
				if (shadowSize == size)
					index = counter;
				counter++;
			}
			element_shadow_list->setStrings(strings2);
			element_shadow_list->setIndex(index);
			element_shadow_list->addCallback(List::on_index_changed, [&, sizes2, qualities, element_shadow_list, engine]() {
				const auto & index = element_shadow_list->getIndex();
				engine->getPreferenceState().setValue(PreferenceState::C_SHADOW_SIZE_DIRECTIONAL, std::min(sizes2[index] * 2, 2048.0f));
				engine->getPreferenceState().setValue(PreferenceState::C_SHADOW_SIZE_POINT, sizes2[index]);
				engine->getPreferenceState().setValue(PreferenceState::C_SHADOW_SIZE_SPOT, sizes2[index]);
				engine->getPreferenceState().setValue(PreferenceState::C_SHADOW_QUALITY, qualities[index]);
			});
			addLabledSetting(graphicsLayout, element_shadow_list, "Shadow Quality:");

			float envSize = 1024;
			engine->getPreferenceState().getOrSetValue(PreferenceState::C_ENVMAP_SIZE, envSize);
			auto element_env_list = std::make_shared<DropList>(engine);
			const std::vector<std::string> strings3 =	{ "Low",	"Medium",	"High",		"Very High",	"Ultra" };
			const std::vector<float> sizes3 =			{ 128.0f,	256.0f,		512.0f,		1024.0f,		2048.0f };
			counter = 0;
			index = 0;
			for each (const auto & size in sizes3) {
				if (envSize == size)
					index = counter;
				counter++;
			}
			element_env_list->setStrings(strings3);
			element_env_list->setIndex(index);
			element_env_list->addCallback(List::on_index_changed, [&, sizes3, element_env_list, engine]() {
				engine->getPreferenceState().setValue(PreferenceState::C_ENVMAP_SIZE, sizes3[element_env_list->getIndex()]);
			});
			addLabledSetting(graphicsLayout, element_env_list, "Reflection Quality:");

			float bounceSize = 1024;
			engine->getPreferenceState().getOrSetValue(PreferenceState::C_RH_BOUNCE_SIZE, bounceSize);
			auto element_bounce_list = std::make_shared<DropList>(engine);
			const std::vector<std::string> strings4 =	{ "Very Low",	"Low",		"Medium",	"High",		"Very High",	"Ultra" };
			const std::vector<float> sizes4 =			{ 8,			12,			16,			24,			32,				64 };
			counter = 0;
			index = 0;
			for each (const auto & size in sizes4) {
				if (bounceSize == size)
					index = counter;
				counter++;
			}
			element_bounce_list->setStrings(strings4);
			element_bounce_list->setIndex(index);
			element_bounce_list->addCallback(List::on_index_changed, [&, sizes4, element_bounce_list, engine]() {
				engine->getPreferenceState().setValue(PreferenceState::C_RH_BOUNCE_SIZE, sizes4[element_bounce_list->getIndex()]);
			});
			addLabledSetting(graphicsLayout, element_bounce_list, "Light Bounce Quality:");

			bool element_bloom_state = true;
			engine->getPreferenceState().getOrSetValue<bool>(PreferenceState::C_BLOOM, element_bloom_state);
			auto element_bloom = std::make_shared<Toggle>(engine, element_bloom_state);
			element_bloom->addCallback(Toggle::on_toggle, [&, element_bloom, engine]() {
				engine->getPreferenceState().setValue(PreferenceState::C_BLOOM, element_bloom->getToggled() ? 1.0f : 0.0f);
			});
			addLabledSetting(graphicsLayout, element_bloom, "Bloom:");

			bool element_ssao_state = true;
			engine->getPreferenceState().getOrSetValue<bool>(PreferenceState::C_SSAO, element_ssao_state);
			auto element_ssao = std::make_shared<Toggle>(engine, element_ssao_state);
			element_ssao->addCallback(Toggle::on_toggle, [&, element_ssao, engine]() {
				engine->getPreferenceState().setValue(PreferenceState::C_SSAO, element_ssao->getToggled() ? 1.0f : 0.0f);
			});
			addLabledSetting(graphicsLayout, element_ssao, "SSAO:");

			bool element_ssr_state = true;
			engine->getPreferenceState().getOrSetValue<bool>(PreferenceState::C_SSR, element_ssr_state);
			auto element_ssr = std::make_shared<Toggle>(engine, element_ssr_state);
			element_ssr->addCallback(Toggle::on_toggle, [&, element_ssr, engine]() {
				engine->getPreferenceState().setValue(PreferenceState::C_SSR, element_ssr->getToggled() ? 1.0f : 0.0f);
			});
			addLabledSetting(graphicsLayout, element_ssr, "SSR:");

			bool element_fxaa_state = true;
			engine->getPreferenceState().getOrSetValue<bool>(PreferenceState::C_FXAA, element_fxaa_state);
			auto element_fxaa = std::make_shared<Toggle>(engine, element_fxaa_state);
			element_fxaa->addCallback(Toggle::on_toggle, [&, element_fxaa, engine]() {
				engine->getPreferenceState().setValue(PreferenceState::C_FXAA, element_fxaa->getToggled() ? 1.0f : 0.0f);
			});
			addLabledSetting(graphicsLayout, element_fxaa, "FXAA:");
		}

		auto bottomBar = std::make_shared<Layout_Horizontal>(engine);
		bottomBar->setMaxScale(glm::vec2(bottomBar->getMaxScale().x, 30.0f));
		TopDownLayout->addElement(bottomBar);

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
	std::vector<glm::ivec3> m_resolutions;
};

#endif // OPTIONSMENU_H