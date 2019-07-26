#include "Modules/Editor/UI/Prefabs.h"
#include "Modules/Editor/Editor_M.h"
#include "Modules/UI/dear imgui/imgui.h"
#include "Engine.h"


Prefabs::Prefabs(Engine * engine, LevelEditor_Module * editor)
	: m_editor(editor)
{
	auto & preferences = engine->getPreferenceState();
	preferences.getOrSetValue(PreferenceState::C_WINDOW_WIDTH, m_renderSize.x);
	preferences.getOrSetValue(PreferenceState::C_WINDOW_HEIGHT, m_renderSize.y);
	preferences.addCallback(PreferenceState::C_WINDOW_WIDTH, m_aliveIndicator, [&](const float &f) {
		m_renderSize.x = (int)f;
	});
	preferences.addCallback(PreferenceState::C_WINDOW_HEIGHT, m_aliveIndicator, [&](const float &f) {
		m_renderSize.y = (int)f;
	});
}

void Prefabs::tick(const float & deltaTime)
{
	bool t = true;
	ImGui::SetNextWindowSize({ 300.0f, m_renderSize.y - 18.0f }, ImGuiCond_Appearing);
	ImGui::SetNextWindowPos({ 0, 18.0f }, ImGuiCond_Appearing);
	if (ImGui::Begin("Prefabs", NULL)) {

	}
	ImGui::End();
}