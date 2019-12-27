#include "Modules/Editor/UI/RecoverDialogue.h"
#include "Engine.h"
#include "imgui.h"


RecoverDialogue::RecoverDialogue(Engine& engine, LevelEditor_Module& editor) noexcept :
	m_engine(engine),
	m_editor(editor)
{
	m_open = false;
}

void RecoverDialogue::tick(const float& /*deltaTime*/)
{
	tickMainDialogue();
}

void RecoverDialogue::setPath(const std::filesystem::path& path)
{
	m_recoveredPath = path;
}

void RecoverDialogue::tickMainDialogue()
{
	if (m_open) {
		ImGui::OpenPopup("Recover Level");
		ImGui::SetNextWindowSize({ 450, 125 }, ImGuiCond_Appearing);
		if (ImGui::BeginPopupModal("Recover Level", &m_open, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove)) {
			const auto message = "The following map file has been recovered:\r\n\t"
				+ std::filesystem::relative(m_recoveredPath, Engine::Get_Current_Dir() + "\\Maps\\").filename().string() + "\r\n\r\n"
				+ "Do you want to open it, ignore it, or delete it?";
			ImGui::TextWrapped(message.c_str());
			ImGui::Spacing();
			ImGui::Separator();
			ImGui::Spacing();

			// Display an open button
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(ImColor::HSV(2.0F / 7.0F, 0.6F, 0.6F)));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(ImColor::HSV(2.0F / 7.0F, 0.7F, 0.7F)));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(ImColor::HSV(2.0F / 7.0F, 0.8F, 0.8F)));
			if (ImGui::Button("Open", { 75, 20 })) {
				const auto relative = std::filesystem::relative(m_recoveredPath, Engine::Get_Current_Dir() + "\\Maps\\").filename().string();
				m_editor.openLevel(relative);
				ImGui::CloseCurrentPopup();
				m_open = false;
			}
			ImGui::PopStyleColor(3);

			// Display a cancel button
			ImGui::SameLine(175.5F);
			if (ImGui::Button("Cancel", { 100, 20 })) {
				ImGui::CloseCurrentPopup();
				m_open = false;
			}
			ImGui::SetItemDefaultFocus();

			// Display a delete button
			ImGui::SameLine(368);
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(ImColor::HSV(0, 0.6F, 0.6F)));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(ImColor::HSV(0, 0.7F, 0.7F)));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(ImColor::HSV(0, 0.8F, 0.8F)));
			if (ImGui::Button("Delete", { 75, 20 }))
				ImGui::OpenPopup("Confirm Delete");
			ImGui::PopStyleColor(3);
			tickDeleteDialogue();
			ImGui::EndPopup();
		}
	}
}

void RecoverDialogue::tickDeleteDialogue()
{
	bool openDelete = true;
	ImGui::SetNextWindowSize({ 350, 95 }, ImGuiCond_Appearing);
	if (ImGui::BeginPopupModal("Confirm Delete", &openDelete, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove)) {
		ImGui::TextWrapped("Are you sure you want to delete this item?\r\nThis action is irreversible.\r\n");
		ImGui::Spacing(); ImGui::Spacing();
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(ImColor::HSV(0, 0.6F, 0.6F)));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(ImColor::HSV(0, 0.7F, 0.7F)));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(ImColor::HSV(0, 0.8F, 0.8F)));
		if (ImGui::Button("Delete", { 75, 20 })) {
			std::error_code ec;
			std::filesystem::remove_all(m_recoveredPath, ec);
			ImGui::CloseCurrentPopup();
			m_open = false;
		}
		ImGui::PopStyleColor(3);
		ImGui::SameLine();
		ImGui::Spacing();
		ImGui::SameLine();
		if (ImGui::Button("Cancel", { 100, 20 }))
			ImGui::CloseCurrentPopup();
		ImGui::SetItemDefaultFocus();
		ImGui::EndPopup();
	}
}