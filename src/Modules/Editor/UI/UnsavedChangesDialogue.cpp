#include "Modules/Editor/UI/UnsavedChangesDialogue.h"
#include "Modules/Editor/Editor_M.h"
#include "imgui.h"


UnsavedChangesDialogue::UnsavedChangesDialogue(Engine& engine, LevelEditor_Module& editor) noexcept :
	m_engine(engine),
	m_editor(editor)
{
	m_open = false;
}

void UnsavedChangesDialogue::tick(const float& /*deltaTime*/)
{
	// Draw 'Delete Level' confirmation
	if (m_open) {
		ImGui::OpenPopup("Unsaved Changes");
		ImGui::SetNextWindowSize({ 350, 80 }, ImGuiCond_Appearing);
		if (ImGui::BeginPopupModal("Unsaved Changes", &m_open, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove)) {
			ImGui::TextWrapped("This level has unsaved changes, discard them?\r\n");
			ImGui::Spacing(); ImGui::Spacing();
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(ImColor::HSV(0, 0.6F, 0.6F)));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(ImColor::HSV(0, 0.7F, 0.7F)));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(ImColor::HSV(0, 0.8F, 0.8F)));
			if (ImGui::Button("Discard", { 66, 20 })) {
				m_open = false;
				ImGui::CloseCurrentPopup();
				executeFunction();
			}
			ImGui::PopStyleColor(3);
			ImGui::SameLine(100, 50);
			if (ImGui::Button("Go Back", { 90, 20 })) {
				m_open = false;
				ImGui::CloseCurrentPopup();
			}
			ImGui::SameLine();
			ImGui::SetItemDefaultFocus();
			if (ImGui::Button("Save", { 90, 20 })) {
				m_open = false;
				ImGui::CloseCurrentPopup();
				m_editor.saveLevel();
				executeFunction();
			}
			ImGui::EndPopup();
		}
	}
}

void UnsavedChangesDialogue::tryPrompt(const std::function<void()>& funcAfterPrompt)
{
	m_open = m_editor.hasUnsavedChanges();
	m_func = funcAfterPrompt;

	if (!m_open)
		executeFunction();
}

void UnsavedChangesDialogue::executeFunction()
{
	if (m_func) {
		m_func();
		m_func = {};
	}
}