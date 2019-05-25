#pragma once
#ifndef OPTIONS_GRAPICS_H
#define OPTIONS_GRAPICS_H

#include "Modules/UI/Macro Elements/Options_Pane.h"
#include "Modules/UI/Basic Elements/Button.h"
#include "Modules/UI/Basic Elements/SideList.h"
#include "Modules/UI/Basic Elements/Slider.h"
#include "Modules/UI/Basic Elements/TextInput.h"
#include "Modules/UI/Basic Elements/Toggle.h"
#include "Engine.h"


/** A UI element serving as a graphics options menu. */
class Options_Graphics : public Options_Pane {
public:
	// Public (de)Constructors
	/** Destroy the graphics panel. */
	inline ~Options_Graphics() = default;
	/** Construct a graphics panel. */
	inline Options_Graphics(Engine * engine) : Options_Pane(engine) {
		// Title
		m_title->setText("Graphics Options");

		// Add Options
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
		addOption(engine, element_material_list, "Texture Quality:", "Adjusts the resolution of in-game geometry textures.");

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
		addOption(engine, element_shadow_list, "Shadow Quality:", "Adjusts the resolution of all dynamic light shadows textures.");

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
		addOption(engine, element_env_list, "Reflection Quality:", "Adjusts the resolution of all environment map textures.");

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
		addOption(engine, element_bounce_list, "Light Bounce Quality:", "Adjusts the resolution of the real-time GI simulation.");

		bool element_bloom_state = true;
		engine->getPreferenceState().getOrSetValue<bool>(PreferenceState::C_BLOOM, element_bloom_state);
		auto element_bloom = std::make_shared<Toggle>(engine, element_bloom_state);
		element_bloom->addCallback(Toggle::on_toggle, [&, element_bloom, engine]() {
			engine->getPreferenceState().setValue(PreferenceState::C_BLOOM, element_bloom->getToggled() ? 1.0f : 0.0f);
		});
		addOption(engine, element_bloom, "Bloom:", "Turns the bloom effect on or off.");

		bool element_ssao_state = true;
		engine->getPreferenceState().getOrSetValue<bool>(PreferenceState::C_SSAO, element_ssao_state);
		auto element_ssao = std::make_shared<Toggle>(engine, element_ssao_state);
		element_ssao->addCallback(Toggle::on_toggle, [&, element_ssao, engine]() {
			engine->getPreferenceState().setValue(PreferenceState::C_SSAO, element_ssao->getToggled() ? 1.0f : 0.0f);
		});
		addOption(engine, element_ssao, "SSAO:", "Turns screen-space ambient occlusion effect on or off. Works with baked AO.");

		bool element_ssr_state = true;
		engine->getPreferenceState().getOrSetValue<bool>(PreferenceState::C_SSR, element_ssr_state);
		auto element_ssr = std::make_shared<Toggle>(engine, element_ssr_state);
		element_ssr->addCallback(Toggle::on_toggle, [&, element_ssr, engine]() {
			engine->getPreferenceState().setValue(PreferenceState::C_SSR, element_ssr->getToggled() ? 1.0f : 0.0f);
		});
		addOption(engine, element_ssr, "SSR:", "Turns screen-space reflections on or off. Works with baked reflections.");

		bool element_fxaa_state = true;
		engine->getPreferenceState().getOrSetValue<bool>(PreferenceState::C_FXAA, element_fxaa_state);
		auto element_fxaa = std::make_shared<Toggle>(engine, element_fxaa_state);
		element_fxaa->addCallback(Toggle::on_toggle, [&, element_fxaa, engine]() {
			engine->getPreferenceState().setValue(PreferenceState::C_FXAA, element_fxaa->getToggled() ? 1.0f : 0.0f);
		});
		addOption(engine, element_fxaa, "FXAA:", "Turns fast approximate anti-aliasing on or off.");
	}
};

#endif // OPTIONS_GRAPICS_H