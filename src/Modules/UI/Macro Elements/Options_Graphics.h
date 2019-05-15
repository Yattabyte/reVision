#pragma once
#ifndef OPTIONS_GRAPICS_H
#define OPTIONS_GRAPICS_H

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


/** A UI element serving as a graphics options menu. */
class Options_Graphics : public UI_Element
{
public:
	// (de)Constructors
	inline ~Options_Graphics() = default;
	inline Options_Graphics(Engine * engine) : UI_Element() {
		// Make a background panel for cosemetic purposes
		auto panel = std::make_shared<Panel>(engine);
		panel->setColor({ 0.1, 0.1, 0.1 });
		m_backPanel = panel;
		addElement(panel);

		// Make a vertical layout to house list items
		auto layout = std::make_shared<Layout_Vertical>();
		layout->setSpacing(1.0f);
		layout->setMargin(5.0f);
		m_layout = layout;
		m_backPanel->addElement(layout);

		// Title
		auto title = std::make_shared<Label>(engine, "Graphics Options");
		title->setTextScale(20.0f);
		title->setAlignment(Label::align_left);
		m_title = title;
		m_backPanel->addElement(title);

		const auto addLabledSetting = [&, engine](auto & element, const std::string & text) {
			auto horizontalLayout = std::make_shared<Layout_Horizontal>();
			horizontalLayout->addElement(std::make_shared<Label>(engine, text));
			horizontalLayout->addElement(element);
			layout->addElement(horizontalLayout);
		};

		float materialSize = 1024;
		engine->getPreferenceState().getOrSetValue(PreferenceState::C_MATERIAL_SIZE, materialSize);
		auto element_material_list = std::make_shared<SideList>(engine);
		element_material_list->setMaxScale(glm::vec2(element_material_list->getMaxScale().x, 12.5f));
		const std::vector<std::string> strings = { "Low",	"Medium",	"High",		"Very High",	"Ultra" };
		const std::vector<float> sizes = { 128.0f,	256.0f,		512.0f,		1024.0f,		2048.0f };
		int counter = 0, index = 0;
		for each (const auto & size in sizes) {
			if (materialSize == size)
				index = counter;
			counter++;
		}
		element_material_list->setStrings(strings);
		element_material_list->setIndex(index);
		element_material_list->addCallback(SideList::on_index_changed, [&, sizes, element_material_list, engine]() {
			engine->getPreferenceState().setValue(PreferenceState::C_MATERIAL_SIZE, sizes[element_material_list->getIndex()]);
		});
		addLabledSetting(element_material_list, "Texture Quality:");

		float shadowSize = 1024, shadowQuality = 4;
		engine->getPreferenceState().getOrSetValue(PreferenceState::C_SHADOW_SIZE_SPOT, shadowSize);
		engine->getPreferenceState().getOrSetValue(PreferenceState::C_SHADOW_QUALITY, shadowQuality);
		auto element_shadow_list = std::make_shared<SideList>(engine);
		element_shadow_list->setMaxScale(glm::vec2(element_shadow_list->getMaxScale().x, 12.5f));
		const std::vector<std::string> strings2 = { "Low",	"Medium",	"High",		"Very High",	"Ultra" };
		const std::vector<float> sizes2 = { 128.0f,	256.0f,		512.0f,		1024.0f,		2048.0f };
		const std::vector<float> qualities = { 1,		2,			3,			4,				5 };
		counter = 0;
		index = 0;
		for each (const auto & size in sizes2) {
			if (shadowSize == size)
				index = counter;
			counter++;
		}
		element_shadow_list->setStrings(strings2);
		element_shadow_list->setIndex(index);
		element_shadow_list->addCallback(SideList::on_index_changed, [&, sizes2, qualities, element_shadow_list, engine]() {
			const auto & index = element_shadow_list->getIndex();
			engine->getPreferenceState().setValue(PreferenceState::C_SHADOW_SIZE_DIRECTIONAL, std::min(sizes2[index] * 2, 2048.0f));
			engine->getPreferenceState().setValue(PreferenceState::C_SHADOW_SIZE_POINT, sizes2[index]);
			engine->getPreferenceState().setValue(PreferenceState::C_SHADOW_SIZE_SPOT, sizes2[index]);
			engine->getPreferenceState().setValue(PreferenceState::C_SHADOW_QUALITY, qualities[index]);
		});
		addLabledSetting(element_shadow_list, "Shadow Quality:");

		float envSize = 1024;
		engine->getPreferenceState().getOrSetValue(PreferenceState::C_ENVMAP_SIZE, envSize);
		auto element_env_list = std::make_shared<SideList>(engine);
		element_env_list->setMaxScale(glm::vec2(element_env_list->getMaxScale().x, 12.5f));
		const std::vector<std::string> strings3 = { "Low",	"Medium",	"High",		"Very High",	"Ultra" };
		const std::vector<float> sizes3 = { 128.0f,	256.0f,		512.0f,		1024.0f,		2048.0f };
		counter = 0;
		index = 0;
		for each (const auto & size in sizes3) {
			if (envSize == size)
				index = counter;
			counter++;
		}
		element_env_list->setStrings(strings3);
		element_env_list->setIndex(index);
		element_env_list->addCallback(SideList::on_index_changed, [&, sizes3, element_env_list, engine]() {
			engine->getPreferenceState().setValue(PreferenceState::C_ENVMAP_SIZE, sizes3[element_env_list->getIndex()]);
		});
		addLabledSetting(element_env_list, "Reflection Quality:");

		float bounceSize = 1024;
		engine->getPreferenceState().getOrSetValue(PreferenceState::C_RH_BOUNCE_SIZE, bounceSize);
		auto element_bounce_list = std::make_shared<SideList>(engine);
		element_bounce_list->setMaxScale(glm::vec2(element_bounce_list->getMaxScale().x, 12.5f));
		const std::vector<std::string> strings4 = { "Very Low",	"Low",		"Medium",	"High",		"Very High",	"Ultra" };
		const std::vector<float> sizes4 = { 8,			12,			16,			24,			32,				64 };
		counter = 0;
		index = 0;
		for each (const auto & size in sizes4) {
			if (bounceSize == size)
				index = counter;
			counter++;
		}
		element_bounce_list->setStrings(strings4);
		element_bounce_list->setIndex(index);
		element_bounce_list->addCallback(SideList::on_index_changed, [&, sizes4, element_bounce_list, engine]() {
			engine->getPreferenceState().setValue(PreferenceState::C_RH_BOUNCE_SIZE, sizes4[element_bounce_list->getIndex()]);
		});
		addLabledSetting(element_bounce_list, "Light Bounce Quality:");

		bool element_bloom_state = true;
		engine->getPreferenceState().getOrSetValue<bool>(PreferenceState::C_BLOOM, element_bloom_state);
		auto element_bloom = std::make_shared<Toggle>(engine, element_bloom_state);
		element_bloom->addCallback(Toggle::on_toggle, [&, element_bloom, engine]() {
			engine->getPreferenceState().setValue(PreferenceState::C_BLOOM, element_bloom->getToggled() ? 1.0f : 0.0f);
		});
		addLabledSetting(element_bloom, "Bloom:");

		bool element_ssao_state = true;
		engine->getPreferenceState().getOrSetValue<bool>(PreferenceState::C_SSAO, element_ssao_state);
		auto element_ssao = std::make_shared<Toggle>(engine, element_ssao_state);
		element_ssao->addCallback(Toggle::on_toggle, [&, element_ssao, engine]() {
			engine->getPreferenceState().setValue(PreferenceState::C_SSAO, element_ssao->getToggled() ? 1.0f : 0.0f);
		});
		addLabledSetting(element_ssao, "SSAO:");

		bool element_ssr_state = true;
		engine->getPreferenceState().getOrSetValue<bool>(PreferenceState::C_SSR, element_ssr_state);
		auto element_ssr = std::make_shared<Toggle>(engine, element_ssr_state);
		element_ssr->addCallback(Toggle::on_toggle, [&, element_ssr, engine]() {
			engine->getPreferenceState().setValue(PreferenceState::C_SSR, element_ssr->getToggled() ? 1.0f : 0.0f);
		});
		addLabledSetting(element_ssr, "SSR:");

		bool element_fxaa_state = true;
		engine->getPreferenceState().getOrSetValue<bool>(PreferenceState::C_FXAA, element_fxaa_state);
		auto element_fxaa = std::make_shared<Toggle>(engine, element_fxaa_state);
		element_fxaa->addCallback(Toggle::on_toggle, [&, element_fxaa, engine]() {
			engine->getPreferenceState().setValue(PreferenceState::C_FXAA, element_fxaa->getToggled() ? 1.0f : 0.0f);
		});
		addLabledSetting(element_fxaa, "FXAA:");
	}


	// Public Interface Implementations
	inline virtual void setScale(const glm::vec2 & scale) override {
		UI_Element::setScale(scale);
		m_backPanel->setScale(scale);
		m_layout->setScale(scale - glm::vec2(50));
		m_layout->setPosition({ 0, -50 });
		m_title->setPosition({ -scale.x + 50, scale.y - 50 });
		enactCallback(on_resize);
	}


private:
	// Private Attributes
	std::vector<glm::ivec3> m_resolutions;
	std::shared_ptr<UI_Element> m_backPanel, m_title, m_layout;
};

#endif // OPTIONS_GRAPICS_H