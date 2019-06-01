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
class Options_Video : public Options_Pane {
public:
	// Public (de)Constructors
	/** Destroy the video pane. */
	inline ~Options_Video() = default;
	/** Contsruct a video pane. 
	@param	engine		the engine to use. */
	inline Options_Video(Engine * engine, UI_Element * parent = nullptr)
		: Options_Pane(engine, parent) {
		// Title
		m_title->setText("Video Options");

		// Resolution Option
		auto element_res = std::make_shared<SideList>(engine, this);
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
		addOption(engine, element_res, "Resolution:", "Changes the resolution the game renders at.", SideList::on_index_changed, [&, element_res, engine]() {
			const auto & index = element_res->getIndex();
			engine->getPreferenceState().setValue(PreferenceState::C_WINDOW_WIDTH, m_resolutions[index].x);
			engine->getPreferenceState().setValue(PreferenceState::C_WINDOW_HEIGHT, m_resolutions[index].y);
			engine->getPreferenceState().setValue(PreferenceState::C_WINDOW_REFRESH_RATE, m_resolutions[index].z);
		});

		// Gamma Option
		float gamma = 1.0f;
		engine->getPreferenceState().getOrSetValue(PreferenceState::C_GAMMA, gamma);
		auto gamma_slider = std::make_shared<Slider>(engine, gamma, glm::vec2(0.0f, 2.0f), this);
		std::ostringstream out;
		out.precision(2);
		out << std::fixed << gamma;
		gamma_slider->setText(out.str());
		addOption(engine, gamma_slider, "Gamma:", "Changes the gamma correction value used.", Slider::on_value_change, [&, gamma_slider, engine]() {
			// Get a round version of the input
			const float round_value = (int)(gamma_slider->getValue() * 100.0f + .5f) / 100.0f;
			std::ostringstream out;
			out.precision(2);
			out << std::fixed << gamma_slider->getValue();
			engine->getPreferenceState().setValue(PreferenceState::C_GAMMA, round_value);
			gamma_slider->setText(out.str());
		});

		// Draw Distance Option
		float ddistance = 1000.0f;
		engine->getPreferenceState().getOrSetValue(PreferenceState::C_DRAW_DISTANCE, ddistance);
		auto ddistance_slider = std::make_shared<Slider>(engine, ddistance, glm::vec2(0.0f, 1000.0f), this);
		addOption(engine, ddistance_slider, "Draw Distance:", "Changes how far geometry can be seen from.", Slider::on_value_change, [&, ddistance_slider, engine]() {
			engine->getPreferenceState().setValue(PreferenceState::C_DRAW_DISTANCE, ddistance_slider->getValue());
		});

		// FOV Option
		float fov = 90.0f;
		engine->getPreferenceState().getOrSetValue(PreferenceState::C_FOV, fov);
		auto fov_slider = std::make_shared<Slider>(engine, fov, glm::vec2(0.0f, 180), this);
		addOption(engine, fov_slider, "Field of view:", "Changes how wide of an angle the scene can be viewed from.", Slider::on_value_change, [&, fov_slider, engine]() {
			// Get a round version of the input
			const int round_value = (int)std::round<int>(fov_slider->getValue());
			// We store as a float, but we want to ensure round numbers
			engine->getPreferenceState().setValue(PreferenceState::C_FOV, (float)round_value);
		});

		// VSync Option
		bool element_sync_state = true;
		engine->getPreferenceState().getOrSetValue<bool>(PreferenceState::C_VSYNC, element_sync_state);
		auto element_sync = std::make_shared<Toggle>(engine, element_sync_state, this);
		addOption(engine, element_sync, "VSync:", "Lock the game's frame-rate to the monitor's refresh rate.", Toggle::on_toggle, [&, element_sync, engine]() {
			engine->getPreferenceState().setValue(PreferenceState::C_VSYNC, element_sync->getToggled() ? 1.0f : 0.0f);
		});

		// Full Screen Option
		bool element_fs_state = true;
		engine->getPreferenceState().getOrSetValue<bool>(PreferenceState::C_WINDOW_FULLSCREEN, element_fs_state);
		auto element_fs = std::make_shared<Toggle>(engine, element_fs_state, this);
		addOption(engine, element_fs, "Full-screen:", "Render the game full-screen instead of as a window.", Toggle::on_toggle, [&, element_fs, engine]() {
			engine->getPreferenceState().setValue(PreferenceState::C_WINDOW_FULLSCREEN, element_fs->getToggled() ? 1.0f : 0.0f);
		});
	}


protected:
	// Protected Attributes
	std::vector<glm::ivec3> m_resolutions;
};

#endif // OPTIONS_VIDEO_H