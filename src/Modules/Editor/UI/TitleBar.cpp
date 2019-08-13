#include "Modules/Editor/UI/TitleBar.h"
#include "Modules/Editor/Editor_M.h"
#include "Modules/UI/dear imgui/imgui.h"
#include "Engine.h"


TitleBar::TitleBar(Engine * engine, LevelEditor_Module * editor)
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

void TitleBar::tick(const float & deltaTime)
{
	if (ImGui::BeginMainMenuBar()) {
		if (ImGui::BeginMenu("File")) {
			if (ImGui::MenuItem("New Level", "CTRL+N")) { m_editor->newLevel(); }
			ImGui::Separator();
			if (ImGui::MenuItem("Open Level", "CTRL+O")) { m_editor->openLevelDialog(); }
			if (ImGui::BeginMenu("Open Recent", false)) {
				ImGui::EndMenu();
			}
			ImGui::Separator();
			if (ImGui::MenuItem("Save Level", "CTRL+S")) { m_editor->saveLevel(); }
			if (ImGui::MenuItem("Save As")) { m_editor->saveLevelDialog(); }
			ImGui::Separator();
			if (ImGui::MenuItem("Exit", "ALT+F4")) { m_editor->exit(); }
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Edit")) {
			if (ImGui::MenuItem("Undo", "CTRL+Z")) { m_editor->undo(); }
			if (ImGui::MenuItem("Redo", "CTRL+Y", false, false)) { m_editor->redo(); }  // Disabled item
			ImGui::Separator();
			if (ImGui::MenuItem("Select All", "CTRL+A")) { m_editor->selectAll(); }
			const bool hasSelection = m_editor->getSelection().size() ? true : false;
			if (ImGui::MenuItem("Clear Selection", "CTRL+D", nullptr, hasSelection)) { m_editor->clearSelection(); }
			const bool canGroup = m_editor->getSelection().size() > 1 ? true : false;
			if (ImGui::MenuItem("Group Selection", "CTRL+G", nullptr, canGroup)) { m_editor->groupSelection(); }
			if (ImGui::MenuItem("Make Prefab", "CTRL+G", nullptr, hasSelection)) { m_editor->makePrefab(); }
			ImGui::Separator();
			if (ImGui::MenuItem("Cut", "CTRL+X", nullptr, hasSelection)) { m_editor->cutSelection(); }
			if (ImGui::MenuItem("Copy", "CTRL+C", nullptr, hasSelection)) { m_editor->copySelection(); }
			const bool canPaste = m_editor->hasCopy();
			if (ImGui::MenuItem("Paste", "CTRL+V", nullptr, canPaste)) { m_editor->paste(); }
			ImGui::Separator();
			if (ImGui::MenuItem("Delete", "DEL", nullptr, hasSelection)) { m_editor->deleteSelection(); }
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}
}