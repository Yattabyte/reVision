#pragma once
#ifndef OPTIONS_VIDEO_H
#define OPTIONS_VIDEO_H

#include "Modules/UI/Basic Elements/UI_Element.h"
#include "Modules/UI/Basic Elements/Button.h"
#include "Modules/UI/Basic Elements/Label.h"
#include "Modules/UI/Basic Elements/Layout_Horizontal.h"
#include "Modules/UI/Basic Elements/Layout_Vertical.h"
#include "Modules/UI/Basic Elements/Panel.h"
#include "Modules/UI/Basic Elements/SideList.h"
#include "Modules/UI/Basic Elements/Slider.h"
#include "Modules/UI/Basic Elements/TextInput.h"
#include "Modules/UI/Basic Elements/Toggle.h"
#include "Modules/UI/Decorators/Border.h"
#include "Modules/UI/Decorators/Scrollbar_V.h"
#include "Engine.h"


/** A UI element serving as a video options menu. */
class Options_Video : public Layout_Vertical
{
public:
	// UI interaction enums
	enum interact {
		on_back = last_interact_index
	};


	// (de)Constructors
	inline ~Options_Video() = default;
	inline Options_Video(Engine * engine) : Layout_Vertical(engine) {
		setScale(glm::vec2(300, 400));

		// Title
		auto title = std::make_shared<Label>(engine, "Video");
		addElement(title);
		title->setTextScale(20.0f);

		const auto addLabledSetting = [&, engine](auto & layout, auto & element, const std::string & text) {
			auto horizontalLayout = std::make_shared<Layout_Horizontal>(engine);
			horizontalLayout->addElement(std::make_shared<Label>(engine, text));			
			horizontalLayout->addElement(element);
			addElement(horizontalLayout);
		};

		// Video Options
		auto windowLayout = std::make_shared<Layout_Vertical>(engine);
		windowLayout->setMargin(5.0f);
		windowLayout->setSpacing(1.0f);

		auto element_res = std::make_shared<SideList>(engine);
		element_res->setMaxScale(glm::vec2(element_res->getMaxScale().x, 12.5f));
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
		element_res->addCallback(SideList::on_index_changed, [&, element_res, engine]() {
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
			{
				value = std::stof(gamma_tinput->getText());
			}
			catch (const std::exception&)
			{
				value = 0.0f;
			}
			engine->getPreferenceState().setValue(PreferenceState::C_GAMMA, value);
			gamma_slider->setPercentage(value / 2.0f);
		});
		gamma_layout->addElement(gamma_slider);
		gamma_layout->addElement(gamma_tinput);
		addLabledSetting(windowLayout, gamma_layout, "Gamma:");

		auto ddistance_layout = std::make_shared<Layout_Horizontal>(engine);
		auto ddistance_slider = std::make_shared<Slider>(engine);
		auto ddistance_tinput = std::make_shared<TextInput>(engine);
		float ddistance = 1000.0f;
		engine->getPreferenceState().getOrSetValue(PreferenceState::C_DRAW_DISTANCE, ddistance);
		ddistance_layout->setMargin(0);
		ddistance_layout->setMaxScale(glm::vec2(ddistance_layout->getMaxScale().x, 12.5));
		ddistance_slider->setMaxScale(glm::vec2(ddistance_slider->getMaxScale().x, 12.5));
		ddistance_tinput->setMaxScale(glm::vec2(25.0f, 12.5));
		ddistance_tinput->setText(std::to_string(ddistance));
		ddistance_slider->setPercentage(ddistance / 1000.0f);
		ddistance_slider->addCallback(Slider::on_slider_change, [&, ddistance_slider, ddistance_tinput, engine]() {
			const float value = 1000.0f * ddistance_slider->getPercentage();
			engine->getPreferenceState().setValue(PreferenceState::C_DRAW_DISTANCE, value);
			ddistance_tinput->setText(std::to_string(value));
		});
		ddistance_tinput->addCallback(TextInput::on_text_change, [&, ddistance_slider, ddistance_tinput, engine]() {
			float
				value = 1000.0f;
			try
			{
				value = std::stof(ddistance_tinput->getText());
			}
			catch (const std::exception&)
			{
				value = 0.0f;
			}
			engine->getPreferenceState().setValue(PreferenceState::C_DRAW_DISTANCE, value);
			ddistance_slider->setPercentage(value / 1000.0f);
		});
		ddistance_layout->addElement(ddistance_slider);
		ddistance_layout->addElement(ddistance_tinput);
		addLabledSetting(windowLayout, ddistance_layout, "Draw Distance:");

		auto fov_layout = std::make_shared<Layout_Horizontal>(engine);
		auto fov_slider = std::make_shared<Slider>(engine);
		auto fov_tinput = std::make_shared<TextInput>(engine);
		float fov = 90.0f;
		engine->getPreferenceState().getOrSetValue(PreferenceState::C_FOV, fov);
		fov_layout->setMargin(0);
		fov_layout->setMaxScale(glm::vec2(fov_layout->getMaxScale().x, 12.5));
		fov_slider->setMaxScale(glm::vec2(fov_slider->getMaxScale().x, 12.5));
		fov_tinput->setMaxScale(glm::vec2(25.0f, 12.5));
		fov_tinput->setText(std::to_string(fov));
		fov_slider->setPercentage(fov / 180.0f);
		fov_slider->addCallback(Slider::on_slider_change, [&, fov_slider, fov_tinput, engine]() {
			const float value = 180.0f * fov_slider->getPercentage();
			engine->getPreferenceState().setValue(PreferenceState::C_FOV, value);
			fov_tinput->setText(std::to_string(value));
		});
		fov_tinput->addCallback(TextInput::on_text_change, [&, fov_slider, fov_tinput, engine]() {
			float
				value = 90.0F;
			try
			{
				value = std::stof(fov_tinput->getText());
			}
			catch (const std::exception&)
			{
				value = 0.0f;
			}
			engine->getPreferenceState().setValue(PreferenceState::C_FOV, value);
			fov_slider->setPercentage(value / 180.0f);
		});
		fov_layout->addElement(fov_slider);
		fov_layout->addElement(fov_tinput);
		addLabledSetting(windowLayout, fov_layout, "Field of view:");

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


private:
	// Private Attributes
	std::vector<glm::ivec3> m_resolutions;
};

#endif // OPTIONS_VIDEO_H