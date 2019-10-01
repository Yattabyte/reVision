#include "Modules/Editor/UI/MissingFileDialogue.h"
#include "Modules/Editor/Editor_M.h"
#include "imgui.h"
#include "Engine.h"


MissingFileDialogue::MissingFileDialogue(Engine* engine, LevelEditor_Module* editor)
	: m_engine(engine), m_editor(editor)
{
	m_open = false;
}

void MissingFileDialogue::tick(const float& deltaTime)
{
	// Draw 'Missing File' notification
	if (m_open && !m_fileName.empty()) {
		ImGui::OpenPopup("Cannot Open File");
		ImGui::SetNextWindowSize({ 400, 105 }, ImGuiCond_Appearing);
		if (ImGui::BeginPopupModal("Cannot Open File", &m_open, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove)) {
			ImGui::TextWrapped("The following file is corrupted or does not exist:");
			ImGui::Spacing();
			ImGui::Indent();
			ImGui::TextWrapped(m_fileName.c_str());
			ImGui::Spacing(); ImGui::Spacing();
			ImGui::Unindent();
			if (ImGui::Button("OK", { 90, 20 })) {
				m_open = false;
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}
	}
}

void MissingFileDialogue::notifyMissing(const std::string& filename)
{
	m_fileName = filename;
	m_open = !filename.empty();
}