#include "Modules/Editor/UI/Inspector.h"
#include "Modules/Editor/Editor_M.h"
#include "Modules/UI/dear imgui/imgui.h"
#include "Engine.h"


Inspector::Inspector(Engine * engine, LevelEditor_Module * editor)
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

void Inspector::render(const float & deltaTime)
{
	bool t = true;
	ImGui::SetNextWindowSize({ 300.0f, m_renderSize.y - 18.0f }, ImGuiCond_Appearing);
	ImGui::SetNextWindowPos({ m_renderSize.x - 300.0f, 18.0f }, ImGuiCond_Appearing);
	if (ImGui::Begin("Inspector", NULL)) {
		
	}
	ImGui::End();
}