#include "Modules/UI/Macro Elements/Options_Graphics.h"
#include "Modules/UI/Basic Elements/Slider.h"
#include "Modules/UI/Basic Elements/SideList.h"
#include "Modules/UI/Basic Elements/Toggle.h"
#include "Engine.h"


Options_Graphics::Options_Graphics(Engine& engine) noexcept :
	Options_Pane(engine)
{
	// Title
	m_title->setText("Graphics Options");

	// Material Size Option
	float materialSize = 1024;
	engine.getPreferenceState().getOrSetValue(PreferenceState::Preference::C_MATERIAL_SIZE, materialSize);
	auto element_material_list = std::make_shared<SideList>(engine);
	element_material_list->setStrings({ "Low",	"Medium",	"High",		"Very High",	"Ultra" });
	m_materialSizes = { 128.0f,		256.0f,		512.0f,		1024.0f,	2048.0f };
	int counter = 0, index = 0;
	for (const auto& size : m_materialSizes) {
		if (materialSize == size)
			index = counter;
		counter++;
	}
	element_material_list->setIndex(index);
	addOption(engine, element_material_list, 1.0f, "Texture Quality:", "Adjusts the resolution of in-game geometry textures.", (int)SideList::Interact::on_index_changed, [&, element_material_list]() { setTextureResolution(element_material_list->getIndex()); });

	// Shadow Size Option
	float shadowSize = 1024;
	engine.getPreferenceState().getOrSetValue(PreferenceState::Preference::C_SHADOW_SIZE, shadowSize);
	auto element_shadow_list = std::make_shared<SideList>(engine);
	element_shadow_list->setStrings({ "Low",	"Medium",	"High",		"Very High",	"Ultra" });
	m_shadowSizes = { 128.0f,	256.0f,		512.0f,		1024.0f,		2048.0f };
	counter = 0;
	index = 0;
	for (const auto& size : m_shadowSizes) {
		if (shadowSize == size)
			index = counter;
		counter++;
	}
	element_shadow_list->setIndex(index);
	addOption(engine, element_shadow_list, 1.0f, "Shadow Quality:", "Adjusts the resolution of all dynamic light shadows textures.", (int)SideList::Interact::on_index_changed, [&, element_shadow_list]() { setShadowSize(element_shadow_list->getIndex()); });

	// Reflection Size Option
	float envSize = 1024;
	engine.getPreferenceState().getOrSetValue(PreferenceState::Preference::C_ENVMAP_SIZE, envSize);
	auto element_env_list = std::make_shared<SideList>(engine);
	element_env_list->setStrings({ "Low",	"Medium",	"High",		"Very High",	"Ultra" });
	m_reflectionSizes = { 128.0f,	256.0f,		512.0f,		1024.0f,		2048.0f };
	counter = 0;
	index = 0;
	for (const auto& size : m_reflectionSizes) {
		if (envSize == size)
			index = counter;
		counter++;
	}
	element_env_list->setIndex(index);
	addOption(engine, element_env_list, 1.0f, "Reflection Quality:", "Adjusts the resolution of all environment map textures.", (int)SideList::Interact::on_index_changed, [&, element_env_list]() { setReflectionSize(element_env_list->getIndex()); });

	// Light Bounce Option
	float bounceSize = 1024;
	engine.getPreferenceState().getOrSetValue(PreferenceState::Preference::C_RH_BOUNCE_SIZE, bounceSize);
	auto element_bounce_list = std::make_shared<SideList>(engine);
	element_bounce_list->setStrings({ "Very Low",	"Low",		"Medium",	"High",		"Very High",	"Ultra" });
	m_bounceQuality = { 8,			12,			16,			24,			32,				64 };
	counter = 0;
	index = 0;
	for (const auto& size : m_bounceQuality) {
		if (bounceSize == size)
			index = counter;
		counter++;
	}
	element_bounce_list->setIndex(index);
	addOption(engine, element_bounce_list, 1.0f, "Light Bounce Quality:", "Adjusts the resolution of the real-time GI simulation.", (int)SideList::Interact::on_index_changed, [&, element_bounce_list]() { setBounceQuality(element_bounce_list->getIndex()); });

	// Shadow Count Option
	float maxShadowCasters = 6.0f;
	engine.getPreferenceState().getOrSetValue(PreferenceState::Preference::C_SHADOW_MAX_PER_FRAME, maxShadowCasters);
	auto maxShadow_slider = std::make_shared<Slider>(engine, maxShadowCasters, glm::vec2(1.0f, 100.0f));
	addOption(engine, maxShadow_slider, 0.75f, "Max Concurrent Shadows:", "Set the maximum number of shadows updated per frame.", (int)Slider::Interact::on_value_change, [&, maxShadow_slider]() {
		m_engine.getPreferenceState().setValue(PreferenceState::Preference::C_SHADOW_MAX_PER_FRAME, maxShadow_slider->getValue());
		});

	// Envmap Count Option
	float maxReflectionCasters = 6.0f;
	engine.getPreferenceState().getOrSetValue(PreferenceState::Preference::C_ENVMAP_MAX_PER_FRAME, maxReflectionCasters);
	auto maxReflection_slider = std::make_shared<Slider>(engine, maxReflectionCasters, glm::vec2(1.0f, 100.0f));
	addOption(engine, maxReflection_slider, 0.75f, "Max Concurrent Reflections:", "Set the maximum number of reflections updated per frame.", (int)Slider::Interact::on_value_change, [&, maxReflection_slider]() {
		m_engine.getPreferenceState().setValue(PreferenceState::Preference::C_ENVMAP_MAX_PER_FRAME, maxReflection_slider->getValue());
		});

	// Bloom Option
	bool element_bloom_state = true;
	engine.getPreferenceState().getOrSetValue<bool>(PreferenceState::Preference::C_BLOOM, element_bloom_state);
	auto element_bloom = std::make_shared<Toggle>(engine, element_bloom_state);
	addOption(engine, element_bloom, 0.5f, "Bloom:", "Turns the bloom effect on or off.", (int)Toggle::Interact::on_toggle, [&, element_bloom]() { setBloom(element_bloom->isToggled()); });

	// SSAO Option
	bool element_ssao_state = true;
	engine.getPreferenceState().getOrSetValue<bool>(PreferenceState::Preference::C_SSAO, element_ssao_state);
	auto element_ssao = std::make_shared<Toggle>(engine, element_ssao_state);
	addOption(engine, element_ssao, 0.5f, "SSAO:", "Turns screen-space ambient occlusion effect on or off. Works with baked AO.", (int)Toggle::Interact::on_toggle, [&, element_ssao]() { setSSAO(element_ssao->isToggled()); });

	// SSR Option
	bool element_ssr_state = true;
	engine.getPreferenceState().getOrSetValue<bool>(PreferenceState::Preference::C_SSR, element_ssr_state);
	auto element_ssr = std::make_shared<Toggle>(engine, element_ssr_state);
	addOption(engine, element_ssr, 0.5f, "SSR:", "Turns screen-space reflections on or off. Works with baked reflections.", (int)Toggle::Interact::on_toggle, [&, element_ssr]() { setSSR(element_ssr->isToggled()); });

	// FXAA Option
	bool element_fxaa_state = true;
	engine.getPreferenceState().getOrSetValue<bool>(PreferenceState::Preference::C_FXAA, element_fxaa_state);
	auto element_fxaa = std::make_shared<Toggle>(engine, element_fxaa_state);
	addOption(engine, element_fxaa, 0.5f, "FXAA:", "Turns fast approximate anti-aliasing on or off.", (int)Toggle::Interact::on_toggle, [&, element_fxaa]() { setFXAA(element_fxaa->isToggled()); });
}

void Options_Graphics::setTextureResolution(const size_t& index) noexcept 
{
	m_engine.getPreferenceState().setValue(PreferenceState::Preference::C_MATERIAL_SIZE, m_materialSizes[index]);
}

void Options_Graphics::setShadowSize(const size_t& index) noexcept 
{
	m_engine.getPreferenceState().setValue(PreferenceState::Preference::C_SHADOW_SIZE, m_shadowSizes[index]);
}

void Options_Graphics::setReflectionSize(const size_t& index) noexcept 
{
	m_engine.getPreferenceState().setValue(PreferenceState::Preference::C_ENVMAP_SIZE, m_reflectionSizes[index]);
}

void Options_Graphics::setBounceQuality(const size_t& index) noexcept 
{
	m_engine.getPreferenceState().setValue(PreferenceState::Preference::C_RH_BOUNCE_SIZE, m_bounceQuality[index]);
}

void Options_Graphics::setBloom(const bool& b) noexcept 
{
	m_engine.getPreferenceState().setValue(PreferenceState::Preference::C_BLOOM, b ? 1.0f : 0.0f);
}

void Options_Graphics::setSSAO(const bool& b) noexcept 
{
	m_engine.getPreferenceState().setValue(PreferenceState::Preference::C_SSAO, b ? 1.0f : 0.0f);
}

void Options_Graphics::setSSR(const bool& b) noexcept 
{
	m_engine.getPreferenceState().setValue(PreferenceState::Preference::C_SSR, b ? 1.0f : 0.0f);
}

void Options_Graphics::setFXAA(const bool& b) noexcept 
{
	m_engine.getPreferenceState().setValue(PreferenceState::Preference::C_FXAA, b ? 1.0f : 0.0f);
}