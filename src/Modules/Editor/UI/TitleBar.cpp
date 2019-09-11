#include "Modules/Editor/UI/TitleBar.h"
#include "Modules/Editor/Editor_M.h"
#include "Modules/UI/dear imgui/imgui.h"
#include "Engine.h"


TitleBar::TitleBar(Engine * engine, LevelEditor_Module * editor)
	: m_engine(engine), m_editor(editor)
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

void TitleBar::tick(const float & deltaTime)
{
	if (ImGui::BeginMainMenuBar()) {
		if (ImGui::BeginMenu("File")) {
			if (ImGui::MenuItem("New Level", "CTRL+N")) { m_editor->newLevel(); }
			ImGui::Separator();
			if (ImGui::MenuItem("Open Level", "CTRL+O")) { m_editor->openLevelDialogue(); }
			if (ImGui::BeginMenu("Open Recent", false)) {
				ImGui::EndMenu();
			}
			ImGui::Separator();
			if (ImGui::MenuItem("Save Level", "CTRL+S")) { m_editor->saveLevel(); }
			if (ImGui::MenuItem("Save As")) { m_editor->saveLevelDialogue(); }
			ImGui::Separator();
			if (ImGui::MenuItem("Exit", "ALT+F4")) { m_editor->exit(); }
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Edit")) {
			if (ImGui::MenuItem("Undo", "CTRL+Z", false, m_editor->canUndo())) { m_editor->undo(); }
			if (ImGui::MenuItem("Redo", "CTRL+Y", false, m_editor->canRedo())) { m_editor->redo(); }
			ImGui::Separator();
			if (ImGui::MenuItem("Select All", "CTRL+A")) { m_editor->selectAll(); }
			const bool hasSelection = m_editor->getSelection().size() ? true : false;
			if (ImGui::MenuItem("Clear Selection", "CTRL+D", nullptr, hasSelection)) { m_editor->clearSelection(); }
			const bool canGroup = m_editor->getSelection().size() > 1 ? true : false;
			if (ImGui::MenuItem("Group Selection", "CTRL+G", nullptr, canGroup)) { m_editor->groupSelection(); }
			if (ImGui::MenuItem("Make Prefab", nullptr, hasSelection)) { m_editor->makePrefab(); }
			ImGui::Separator();
			if (ImGui::MenuItem("Cut", "CTRL+X", nullptr, hasSelection)) { m_editor->cutSelection(); }
			if (ImGui::MenuItem("Copy", "CTRL+C", nullptr, hasSelection)) { m_editor->copySelection(); }
			const bool canPaste = m_editor->hasCopy();
			if (ImGui::MenuItem("Paste", "CTRL+V", nullptr, canPaste)) { m_editor->paste(); }
			ImGui::Separator();
			if (ImGui::MenuItem("Delete", "DEL", nullptr, hasSelection)) { m_editor->deleteSelection(); }
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Tools")) {
			static bool qwee = true;
			ImGui::Checkbox("Align-to-grid", &qwee);
			if (qwee) {
				static float asd = 0.0f;
				if (ImGui::SliderFloat("Position", &asd, 0.0f, 100.0f)) {

				}
				if (ImGui::SliderAngle("Rotation", &asd, 0.0f, 100.0f)) {

				}
				if (ImGui::SliderFloat("Scale", &asd, 0.0f, 100.0f)) {

				}
			}
			ImGui::Separator();
			if (ImGui::MenuItem("Settings")) { 

			}
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}

	// Check keyboard input
	const auto& io = ImGui::GetIO();
	const auto pressedKey = [&](const auto& c) -> bool {
		return (io.KeyCtrl && ImGui::IsKeyPressed(c));
	};
	if (pressedKey('N'))
		m_editor->newLevel();
	if (pressedKey('O'))
		m_editor->openLevelDialogue();
	if (pressedKey('S'))
		m_editor->saveLevel();
	if (pressedKey('Z'))
		m_editor->undo();
	if (pressedKey('Y'))
		m_editor->redo();
	if (pressedKey('A'))
		m_editor->selectAll();
	if (pressedKey('D'))
		m_editor->clearSelection();
	if (pressedKey('G'))
		m_editor->groupSelection();
	if (pressedKey('X'))
		m_editor->cutSelection();
	if (pressedKey('C'))
		m_editor->copySelection();
	if (pressedKey('V'))
		m_editor->paste();
	if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Delete)))
		m_editor->deleteSelection();
}