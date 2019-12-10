#include "Modules/Editor/UI/TitleBar.h"
#include "Modules/Editor/Editor_M.h"
#include "Engine.h"
#include "imgui.h"


TitleBar::TitleBar(Engine& engine, LevelEditor_Module& editor) noexcept :
	m_engine(engine),
	m_editor(editor)
{
	m_open = true;

	// Preferences
	auto& preferences = engine.getPreferenceState();
	preferences.getOrSetValue(PreferenceState::Preference::C_WINDOW_WIDTH, m_renderSize.x);
	preferences.getOrSetValue(PreferenceState::Preference::C_WINDOW_HEIGHT, m_renderSize.y);
	preferences.addCallback(PreferenceState::Preference::C_WINDOW_WIDTH, m_aliveIndicator, [&](const float& f) noexcept {
		m_renderSize.x = (int)f;
		});
	preferences.addCallback(PreferenceState::Preference::C_WINDOW_HEIGHT, m_aliveIndicator, [&](const float& f) noexcept {
		m_renderSize.y = (int)f;
		});

	// Assets
	m_iconNew = Shared_Texture(engine, "Editor//iconNew.png");
	m_iconOpen = Shared_Texture(engine, "Editor//iconOpen.png");
	m_iconRecent = Shared_Texture(engine, "Editor//iconOpenRecent.png");
	m_iconSave = Shared_Texture(engine, "Editor//iconSave.png");
	m_iconSaveAs = Shared_Texture(engine, "Editor//iconSaveAs.png");
	m_iconExit = Shared_Texture(engine, "Editor//iconQuit.png");
	m_iconUndo = Shared_Texture(engine, "Editor//iconUndo.png");
	m_iconRedo = Shared_Texture(engine, "Editor//iconRedo.png");
	m_iconCut = Shared_Texture(engine, "Editor//iconCut.png");
	m_iconCopy = Shared_Texture(engine, "Editor//iconCopy.png");
	m_iconPaste = Shared_Texture(engine, "Editor//iconPaste.png");
	m_iconDelete = Shared_Texture(engine, "Editor//iconDelete.png");
	m_iconSettings = Shared_Texture(engine, "Editor//iconOptions.png");
}

void TitleBar::tick(const float&) noexcept
{
	if (m_open) {
		constexpr static auto BeginMenuWIcon = [](const char* string, const Shared_Texture& iconTexture, const char* shortcut = nullptr, bool* selected = nullptr, const bool& enabled = true) -> bool {
			GLuint icon = iconTexture->ready() ? iconTexture->m_glTexID : 0;
			if (icon != 0u) {
				ImGui::Image((ImTextureID)static_cast<uintptr_t>(icon), ImVec2(15, 15), { 0.0f, 1.0f }, { 1.0f, 0.0f });
				ImGui::SameLine();
			}
			return ImGui::MenuItem(string, shortcut, selected, enabled);
		};
		if (ImGui::BeginMainMenuBar()) {
			if (ImGui::BeginMenu("File")) {
				if (BeginMenuWIcon("New Level", m_iconNew, "CTRL+N")) { m_editor.newLevel(); }
				ImGui::Separator();
				if (BeginMenuWIcon("Open Level", m_iconOpen, "CTRL+O")) { m_editor.openLevelDialogue(); }
				{
					ImGui::Image((ImTextureID)static_cast<uintptr_t>(m_iconRecent->ready() ? m_iconRecent->m_glTexID : 0), ImVec2(15, 15), { 0.0f, 1.0f }, { 1.0f, 0.0f });
					ImGui::SameLine();
					const auto recentLevels = m_editor.getRecentLevels();
					if (ImGui::BeginMenu("Open Recent", recentLevels.size())) {
						for (const auto& levelName : recentLevels) {
							ImGui::PushID(&levelName);
							if (ImGui::MenuItem(levelName.c_str()))
								m_editor.openLevel(levelName);
							ImGui::PopID();
						}
						ImGui::EndMenu();
					}
				}
				ImGui::Separator();
				if (BeginMenuWIcon("Save Level", m_iconSave, "CTRL+S")) { m_editor.saveLevel(); }
				if (BeginMenuWIcon("Save As", m_iconSaveAs)) { m_editor.saveLevelDialogue(); }
				ImGui::Separator();
				if (BeginMenuWIcon("Exit", m_iconExit, "ALT+F4")) { m_editor.exit(); }
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Edit")) {
				if (BeginMenuWIcon("Undo", m_iconUndo, "CTRL+Z", nullptr, m_editor.canUndo())) { m_editor.undo(); }
				if (BeginMenuWIcon("Redo", m_iconRedo, "CTRL+Y", nullptr, m_editor.canRedo())) { m_editor.redo(); }
				ImGui::Separator();
				if (ImGui::MenuItem("Select All", "CTRL+A")) { m_editor.selectAll(); }
				const bool hasSelection = m_editor.getSelection().size() ? true : false;
				if (ImGui::MenuItem("Clear Selection", "CTRL+D", nullptr, hasSelection)) { m_editor.clearSelection(); }
				const bool canGroup = m_editor.getSelection().size() > 1 ? true : false;
				if (ImGui::MenuItem("Group Selection", "CTRL+G", nullptr, canGroup)) { m_editor.groupSelection(); }
				if (ImGui::MenuItem("Make Prefab", nullptr, hasSelection)) { m_editor.makePrefab(); }
				ImGui::Separator();
				if (BeginMenuWIcon("Cut", m_iconCut, "CTRL+X", nullptr, hasSelection)) { m_editor.cutSelection(); }
				if (BeginMenuWIcon("Copy", m_iconCopy, "CTRL+C", nullptr, hasSelection)) { m_editor.copySelection(); }
				const bool canPaste = m_editor.hasCopy();
				if (BeginMenuWIcon("Paste", m_iconPaste, "CTRL+V", nullptr, canPaste)) { m_editor.paste(); }
				ImGui::Separator();
				if (BeginMenuWIcon("Delete", m_iconDelete, "DEL", nullptr, hasSelection)) { m_editor.deleteSelection(); }
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Tools")) {
				if (ImGui::MenuItem("Scene Inspector")) { m_editor.openSceneInspector(); }
				if (ImGui::MenuItem("Entity Inspector")) { m_editor.openEntityInspector(); }
				if (ImGui::MenuItem("Prefabs")) { m_editor.openPrefabs(); }
				ImGui::Separator();
				if (BeginMenuWIcon("Settings", m_iconSettings)) { m_editor.openSettingsDialogue(); }
				ImGui::EndMenu();
			}
			ImGui::EndMainMenuBar();
		}
	}
}