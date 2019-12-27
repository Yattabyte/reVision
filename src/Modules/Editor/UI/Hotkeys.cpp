#include "Modules/Editor/UI/Hotkeys.h"
#include "Modules/Editor/Editor_M.h"
#include "imgui.h"


Hotkeys::Hotkeys(Engine& engine, LevelEditor_Module& editor) noexcept :
	m_engine(engine),
	m_editor(editor)
{
	m_open = true;
}

void Hotkeys::tick(const float& /*deltaTime*/)
{
	if (m_open && !ImGui::IsAnyItemActive()) {
		// Check keyboard input
		const auto& io = ImGui::GetIO();
		const auto pressedKey = [&](const auto& c) -> bool {
			return (io.KeyCtrl && ImGui::IsKeyPressed(c));
		};
		if (pressedKey('N'))
			m_editor.newLevel();
		if (pressedKey('O'))
			m_editor.openLevelDialogue();
		if (pressedKey('S'))
			m_editor.saveLevel();
		if (pressedKey('Z'))
			m_editor.undo();
		if (pressedKey('Y'))
			m_editor.redo();
		if (pressedKey('A'))
			m_editor.selectAll();
		if (pressedKey('D'))
			m_editor.clearSelection();
		if (pressedKey('G'))
			m_editor.groupSelection();
		if (pressedKey('X'))
			m_editor.cutSelection();
		if (pressedKey('C'))
			m_editor.copySelection();
		if (pressedKey('V'))
			m_editor.paste();
		if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Delete)))
			m_editor.deleteSelection();
	}
}