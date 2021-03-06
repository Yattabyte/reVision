#include "Modules/UI/Macro Elements/Options_Video.h"
#include "Modules/UI/Basic Elements/SideList.h"
#include "Modules/UI/Basic Elements/Slider.h"
#include "Modules/UI/Basic Elements/Toggle.h"
#include "Engine.h"
#include <sstream>


Options_Video::Options_Video(Engine& engine) :
	Options_Pane(engine)
{
	// Title
	m_title->setText("Video Options");

	// Resolution Option
	auto element_res = std::make_shared<SideList>(engine);
	float width = 1920.0F;
	float height = 1080.0F;
	engine.getPreferenceState().getOrSetValue(PreferenceState::Preference::C_WINDOW_WIDTH, width);
	engine.getPreferenceState().getOrSetValue(PreferenceState::Preference::C_WINDOW_HEIGHT, height);
	m_resolutions = Window::GetResolutions();
	std::vector<std::string> strings(m_resolutions.size());
	int counter = 0;
	int index = 0;
	for (const auto& res : m_resolutions) {
		const auto string = std::to_string(res.x) + "x" + std::to_string(res.y) + " @ " + std::to_string(res.z) + "Hz";
		strings[counter] = string;
		if (res.x == width && res.y == height)
			index = counter;
		counter++;
	}
	element_res->setStrings(strings);
	element_res->setIndex(index);
	addOption(engine, element_res, 1.0F, "Resolution:", "Changes the resolution the game renders at.", static_cast<int>(SideList::Interact::on_index_changed), [&, element_res]() {
		const auto& resolutionIndex = element_res->getIndex();
		m_engine.getPreferenceState().setValue(PreferenceState::Preference::C_WINDOW_WIDTH, m_resolutions[resolutionIndex].x);
		m_engine.getPreferenceState().setValue(PreferenceState::Preference::C_WINDOW_HEIGHT, m_resolutions[resolutionIndex].y);
		m_engine.getPreferenceState().setValue(PreferenceState::Preference::C_WINDOW_REFRESH_RATE, m_resolutions[resolutionIndex].z);
		});

	// Gamma Option
	float gamma = 1.0F;
	engine.getPreferenceState().getOrSetValue(PreferenceState::Preference::C_GAMMA, gamma);
	auto gamma_slider = std::make_shared<Slider>(engine, gamma, glm::vec2(0.0F, 2.0F));
	std::ostringstream out;
	out.precision(2);
	out << std::fixed << gamma;
	gamma_slider->setText(out.str());
	addOption(engine, gamma_slider, 0.75F, "Gamma:", "Changes the gamma correction value used.", static_cast<int>(Slider::Interact::on_value_change), [&, gamma_slider]() {
		// Get a round version of the input
		const float round_value = static_cast<float>(static_cast<int>(gamma_slider->getValue() * 100.0f + 0.5f)) / 100.0f;
		std::ostringstream sstream;
		sstream.precision(2);
		sstream << std::fixed << gamma_slider->getValue();
		m_engine.getPreferenceState().setValue(PreferenceState::Preference::C_GAMMA, round_value);
		gamma_slider->setText(sstream.str());
		});

	// Draw Distance Option
	float ddistance = 1000.0F;
	engine.getPreferenceState().getOrSetValue(PreferenceState::Preference::C_DRAW_DISTANCE, ddistance);
	const auto ddistance_slider = std::make_shared<Slider>(engine, ddistance, glm::vec2(0.0F, 1000.0F));
	addOption(engine, ddistance_slider, 0.75F, "Draw Distance:", "Changes how far geometry can be seen from.", static_cast<int>(Slider::Interact::on_value_change), [&, ddistance_slider]() {
		m_engine.getPreferenceState().setValue(PreferenceState::Preference::C_DRAW_DISTANCE, ddistance_slider->getValue());
		});

	// FOV Option
	float fov = 90.0F;
	engine.getPreferenceState().getOrSetValue(PreferenceState::Preference::C_FOV, fov);
	const auto fov_slider = std::make_shared<Slider>(engine, fov, glm::vec2(0.0F, 180));
	addOption(engine, fov_slider, 0.75F, "Field of view:", "Changes how wide of an angle the scene can be viewed from.", static_cast<int>(Slider::Interact::on_value_change), [&, fov_slider]() {
		// Get a round version of the input
		const float round_value = static_cast<float>(static_cast<int>(std::round(fov_slider->getValue())));
		// We store as a float, but we want to ensure round numbers
		m_engine.getPreferenceState().setValue(PreferenceState::Preference::C_FOV, round_value);
		});

	// VSync Option
	bool element_sync_state = true;
	engine.getPreferenceState().getOrSetValue<bool>(PreferenceState::Preference::C_VSYNC, element_sync_state);
	const auto element_sync = std::make_shared<Toggle>(engine, element_sync_state);
	addOption(engine, element_sync, 0.5F, "VSync:", "Lock the game's frame-rate to the monitor's refresh rate.", static_cast<int>(Toggle::Interact::on_toggle), [&, element_sync]() {
		m_engine.getPreferenceState().setValue(PreferenceState::Preference::C_VSYNC, element_sync->isToggled() ? 1.0f : 0.0f);
		});

	// Full Screen Option
	bool element_fs_state = true;
	engine.getPreferenceState().getOrSetValue<bool>(PreferenceState::Preference::C_WINDOW_FULLSCREEN, element_fs_state);
	const auto element_fs = std::make_shared<Toggle>(engine, element_fs_state);
	addOption(engine, element_fs, 0.5F, "Full-screen:", "Render the game full-screen instead of as a window.", static_cast<int>(Toggle::Interact::on_toggle), [&, element_fs]() {
		m_engine.getPreferenceState().setValue(PreferenceState::Preference::C_WINDOW_FULLSCREEN, element_fs->isToggled() ? 1.0f : 0.0f);
		});
}