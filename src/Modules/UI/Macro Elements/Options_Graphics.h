#pragma once
#ifndef OPTIONS_GRAPICS_H
#define OPTIONS_GRAPICS_H

#include "Modules/UI/Macro Elements/Options_Pane.h"
#include "Modules/UI/Basic Elements/Button.h"
#include "Modules/UI/Basic Elements/SideList.h"
#include "Modules/UI/Basic Elements/Toggle.h"
#include "Engine.h"


/** A UI element serving as a graphics options menu. */
class Options_Graphics : public Options_Pane {
public:
	// Public (de)Constructors
	/** Destroy the graphics panel. */
	inline ~Options_Graphics() = default;
	/** Construct a graphics panel.
	@param	engine		the engine to use. */
	inline Options_Graphics(Engine * engine)
		: Options_Pane(engine) {
		// Title
		m_title->setText("Graphics Options");

		// Material Size Option
		float materialSize = 1024;
		engine->getPreferenceState().getOrSetValue(PreferenceState::C_MATERIAL_SIZE, materialSize);
		auto element_material_list = std::make_shared<SideList>(engine);
		element_material_list->setStrings({ "Low",	"Medium",	"High",		"Very High",	"Ultra" });
		m_materialSizes = { 128.0f,		256.0f,		512.0f,		1024.0f,	2048.0f };
		int counter = 0, index = 0;
		for each (const auto & size in m_materialSizes) {
			if (materialSize == size)
				index = counter;
			counter++;
		}
		element_material_list->setIndex(index);
		addOption(engine, element_material_list, 1.0f, "Texture Quality:", "Adjusts the resolution of in-game geometry textures.", SideList::on_index_changed, [&, element_material_list]() { setTextureResolution(element_material_list->getIndex()); });

		// Shadow Size Option
		float shadowSize = 1024;
		engine->getPreferenceState().getOrSetValue(PreferenceState::C_SHADOW_SIZE, shadowSize);
		auto element_shadow_list = std::make_shared<SideList>(engine);
		element_shadow_list->setStrings({ "Low",	"Medium",	"High",		"Very High",	"Ultra" });
		m_shadowSizes = { 128.0f,	256.0f,		512.0f,		1024.0f,		2048.0f };
		counter = 0;
		index = 0;
		for each (const auto & size in m_shadowSizes) {
			if (shadowSize == size)
				index = counter;
			counter++;
		}
		element_shadow_list->setIndex(index);
		addOption(engine, element_shadow_list, 1.0f, "Shadow Quality:", "Adjusts the resolution of all dynamic light shadows textures.", SideList::on_index_changed, [&, element_shadow_list]() { setShadowSize(element_shadow_list->getIndex()); });

		// Reflection Size Option
		float envSize = 1024;
		engine->getPreferenceState().getOrSetValue(PreferenceState::C_ENVMAP_SIZE, envSize);
		auto element_env_list = std::make_shared<SideList>(engine);
		element_env_list->setStrings({ "Low",	"Medium",	"High",		"Very High",	"Ultra" });
		m_reflectionSizes = { 128.0f,	256.0f,		512.0f,		1024.0f,		2048.0f };
		counter = 0;
		index = 0;
		for each (const auto & size in m_reflectionSizes) {
			if (envSize == size)
				index = counter;
			counter++;
		}
		element_env_list->setIndex(index);
		addOption(engine, element_env_list, 1.0f, "Reflection Quality:", "Adjusts the resolution of all environment map textures.", SideList::on_index_changed, [&, element_env_list]() { setReflectionSize(element_env_list->getIndex()); });

		// Light Bounce Option
		float bounceSize = 1024;
		engine->getPreferenceState().getOrSetValue(PreferenceState::C_RH_BOUNCE_SIZE, bounceSize);
		auto element_bounce_list = std::make_shared<SideList>(engine);
		element_bounce_list->setStrings({ "Very Low",	"Low",		"Medium",	"High",		"Very High",	"Ultra" });
		m_bounceQuality = { 8,			12,			16,			24,			32,				64 };
		counter = 0;
		index = 0;
		for each (const auto & size in m_bounceQuality) {
			if (bounceSize == size)
				index = counter;
			counter++;
		}
		element_bounce_list->setIndex(index);
		addOption(engine, element_bounce_list, 1.0f, "Light Bounce Quality:", "Adjusts the resolution of the real-time GI simulation.", SideList::on_index_changed, [&, element_bounce_list]() { setBounceQuality(element_bounce_list->getIndex()); });

		// Bloom Option
		bool element_bloom_state = true;
		engine->getPreferenceState().getOrSetValue<bool>(PreferenceState::C_BLOOM, element_bloom_state);
		auto element_bloom = std::make_shared<Toggle>(engine, element_bloom_state);
		addOption(engine, element_bloom, 0.5f, "Bloom:", "Turns the bloom effect on or off.", Toggle::on_toggle, [&, element_bloom]() { setBloom(element_bloom->getToggled()); });

		// SSAO Option
		bool element_ssao_state = true;
		engine->getPreferenceState().getOrSetValue<bool>(PreferenceState::C_SSAO, element_ssao_state);
		auto element_ssao = std::make_shared<Toggle>(engine, element_ssao_state);
		addOption(engine, element_ssao, 0.5f, "SSAO:", "Turns screen-space ambient occlusion effect on or off. Works with baked AO.", Toggle::on_toggle, [&, element_ssao]() { setSSAO(element_ssao->getToggled()); });;

		// SSR Option
		bool element_ssr_state = true;
		engine->getPreferenceState().getOrSetValue<bool>(PreferenceState::C_SSR, element_ssr_state);
		auto element_ssr = std::make_shared<Toggle>(engine, element_ssr_state);
		addOption(engine, element_ssr, 0.5f, "SSR:", "Turns screen-space reflections on or off. Works with baked reflections.", Toggle::on_toggle, [&, element_ssr]() { setSSR(element_ssr->getToggled()); });

		// FXAA Option
		bool element_fxaa_state = true;
		engine->getPreferenceState().getOrSetValue<bool>(PreferenceState::C_FXAA, element_fxaa_state);
		auto element_fxaa = std::make_shared<Toggle>(engine, element_fxaa_state);
		addOption(engine, element_fxaa, 0.5f, "FXAA:", "Turns fast approximate anti-aliasing on or off.", Toggle::on_toggle, [&, element_fxaa]() { setFXAA(element_fxaa->getToggled()); });
	}


protected:
	// Protected Methods
	/** Set the resolution.
	@param	index	the resolution index to use. */
	inline void setTextureResolution(const size_t & index) {
		m_engine->getPreferenceState().setValue(PreferenceState::C_MATERIAL_SIZE, m_materialSizes[index]);
	}
	/** Set the shadow size.
	@param	index	the shadow size index to use. */
	inline void setShadowSize(const size_t & index) {
		m_engine->getPreferenceState().setValue(PreferenceState::C_SHADOW_SIZE, m_shadowSizes[index]);
	}
	/** Set the reflection size.
	@param	index	the reflection size index to use. */
	inline void setReflectionSize(const size_t & index) {
		m_engine->getPreferenceState().setValue(PreferenceState::C_ENVMAP_SIZE, m_reflectionSizes[index]);
	}
	/** Set the light bounce quality.
	@param	index	the light bounce quality index to use. */
	inline void setBounceQuality(const size_t & index) {
		m_engine->getPreferenceState().setValue(PreferenceState::C_RH_BOUNCE_SIZE, m_bounceQuality[index]);
	}
	/** Turn the bloom on or off.
	@param	b		whether to turn bloom on or off. */
	inline void setBloom(const bool & b) {
		m_engine->getPreferenceState().setValue(PreferenceState::C_BLOOM, b ? 1.0f : 0.0f);
	}
	/** Turn the SSAO on or off.
	@param	b		whether to turn SSAO on or off. */
	inline void setSSAO(const bool & b) {
		m_engine->getPreferenceState().setValue(PreferenceState::C_SSAO, b ? 1.0f : 0.0f);
	}
	/** Turn the SSR on or off.
	@param	b		whether to turn SSR on or off. */
	inline void setSSR(const bool & b) {
		m_engine->getPreferenceState().setValue(PreferenceState::C_SSR, b ? 1.0f : 0.0f);
	}
	/** Turn the FXAA on or off.
	@param	b		whether to turn FXAA on or off. */
	inline void setFXAA(const bool & b) {
		m_engine->getPreferenceState().setValue(PreferenceState::C_FXAA, b ? 1.0f : 0.0f);
	}


	// Protected Attributes
	std::vector<float> m_materialSizes, m_shadowSizes, m_reflectionSizes, m_bounceQuality;
};

#endif // OPTIONS_GRAPICS_H