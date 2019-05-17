#pragma once
#ifndef OPTIONS_VIDEO_H
#define OPTIONS_VIDEO_H

#include "Modules/UI/Macro Elements/Options_Pane.h"
#include "Modules/UI/Basic Elements/Button.h"
#include "Modules/UI/Basic Elements/SideList.h"
#include "Modules/UI/Basic Elements/Slider.h"
#include "Modules/UI/Basic Elements/TextInput.h"
#include "Modules/UI/Basic Elements/Toggle.h"
#include "Engine.h"
#include <sstream>


/** A UI element serving as a video options menu. */
class Options_Video : public Options_Pane
{
public:
	// (de)Constructors
	inline ~Options_Video() = default;
	inline Options_Video(Engine * engine) : Options_Pane(engine) {
		// Title
		m_title->setText("Video Options");

		// Add Options
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
		addOption(engine, element_res, "Resolution:", "Changes the resolution the game renders at.");

		float gamma = 1.0f;
		engine->getPreferenceState().getOrSetValue(PreferenceState::C_GAMMA, gamma);
		auto gamma_slider = std::make_shared<Slider>(engine, gamma / 2.0f);
		std::ostringstream out;
		out.precision(2);
		out << std::fixed << gamma;
		gamma_slider->setText(out.str());
		gamma_slider->addCallback(Slider::on_slider_change, [&, gamma_slider, engine]() {
			// Get a round version of the input
			const float value = 2.0f * gamma_slider->getPercentage();
			const float round_value = (int)(value * 100.0f + .5f) / 100.0f;

			std::ostringstream out;
			out.precision(2);
			out << std::fixed << value;
			engine->getPreferenceState().setValue(PreferenceState::C_GAMMA, round_value);
			gamma_slider->setText(out.str());
		});
		addOption(engine, gamma_slider, "Gamma:", "Changes the gamma correction value used.");

		float ddistance = 1000.0f;
		engine->getPreferenceState().getOrSetValue(PreferenceState::C_DRAW_DISTANCE, ddistance);
		auto ddistance_slider = std::make_shared<Slider>(engine, ddistance / 1000.0f);
		ddistance_slider->setText(std::to_string((int)std::round<int>(ddistance)));
		ddistance_slider->addCallback(Slider::on_slider_change, [&, ddistance_slider, engine]() {
			// Get a round version of the input
			const float value = 1000.0f * ddistance_slider->getPercentage();
			const int round_value = (int)std::round<int>(value);
			engine->getPreferenceState().setValue(PreferenceState::C_DRAW_DISTANCE, value);
			ddistance_slider->setText(std::to_string(round_value));
		});
		addOption(engine, ddistance_slider, "Draw Distance:", "Changes how far geometry can be seen from.");

		float fov = 90.0f;
		engine->getPreferenceState().getOrSetValue(PreferenceState::C_FOV, fov);
		auto fov_slider = std::make_shared<Slider>(engine, fov / 180.0f);
		fov_slider->setText(std::to_string((int)std::round<int>(fov)));
		fov_slider->addCallback(Slider::on_slider_change, [&, fov_slider, engine]() {
			// Get a round version of the input
			const float value = 180.0f * fov_slider->getPercentage();
			const int round_value = (int)std::round<int>(value);

			// We store as a float, but we want to ensure round numbers
			engine->getPreferenceState().setValue(PreferenceState::C_FOV, (float)round_value);
			fov_slider->setText(std::to_string(round_value));			
		});
		addOption(engine, fov_slider, "Field of view:", "Changes how wide of an angle the scene can be viewed from.");

		bool element_sync_state = true;
		engine->getPreferenceState().getOrSetValue<bool>(PreferenceState::C_VSYNC, element_sync_state);
		auto element_sync = std::make_shared<Toggle>(engine, element_sync_state);
		element_sync->addCallback(Toggle::on_toggle, [&, element_sync, engine]() {
			engine->getPreferenceState().setValue(PreferenceState::C_VSYNC, element_sync->getToggled() ? 1.0f : 0.0f);
		});
		addOption(engine, element_sync, "VSync:", "Lock the game's frame-rate to the monitor's refresh rate.");

		bool element_fs_state = true;
		engine->getPreferenceState().getOrSetValue<bool>(PreferenceState::C_WINDOW_FULLSCREEN, element_fs_state);
		auto element_fs = std::make_shared<Toggle>(engine, element_fs_state);
		element_fs->addCallback(Toggle::on_toggle, [&, element_fs, engine]() {
			engine->getPreferenceState().setValue(PreferenceState::C_WINDOW_FULLSCREEN, element_fs->getToggled() ? 1.0f : 0.0f);
		});
		addOption(engine, element_fs, "Full-screen:", "Render the game full-screen instead of as a window.");
	}


private:
	// Private Attributes
	std::vector<glm::ivec3> m_resolutions;
};

#endif // OPTIONS_VIDEO_H