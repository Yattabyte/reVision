#include "Modules/Editor/UI/LevelDialogue.h"
#include "Modules/Editor/Editor_M.h"
#include "Modules/UI/dear imgui/imgui.h"
#include "Engine.h"
#include <filesystem>


LevelDialogue::LevelDialogue(Engine* engine, LevelEditor_Module* editor)
	: m_engine(engine), m_editor(editor)
{
}

void LevelDialogue::tick(const float& deltaTime)
{
	static bool freshlyOpened = true; // flag used for operations that should happen only once-per-opening
	const auto title = m_openOrSave ? "Open Level" : "Save Level";
	if (m_popupOpen) {
		ImGui::OpenPopup(title);
	}
	ImGui::SetNextWindowSize({ 600, 500 }, ImGuiCond_Appearing);
	if (ImGui::BeginPopupModal(title, &m_popupOpen, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove)) {
		enum PrefabOptions {
			none,
			use,
			del,
			rename
		} option = none;
		// Display an appropriate message
		if (m_openOrSave == true)
			ImGui::Text("Choose a level to open...");
		else
			ImGui::Text("Choose a name to save the level as...");
		ImGui::Spacing();

		// Display a list of level entries for the directory chosen
		static char nameInput[256];
		static auto setName = [&]() {
			if (m_selected != -1) {
				for (size_t x = 0; x < m_levels[m_selected].name.length() && x < IM_ARRAYSIZE(nameInput); ++x)
					nameInput[x] = m_levels[m_selected].name[x];
				nameInput[std::min(256ull, m_levels[m_selected].name.length())] = '\0';
			}
		};
		if (freshlyOpened) {
			populateLevels();
			setName();
		}
		int index = 0;
		ImGui::BeginChild("Level List", ImVec2(580, ImGui::GetWindowContentRegionMax().y - 110), true);
		for each (const auto & level in m_levels) {
			ImGui::PushID(index);
			ImGui::Selectable(level.name.c_str(), m_selected == index);
			if (ImGui::IsItemClicked()) {
				m_selected = index;
				setName();
				if (ImGui::IsMouseDoubleClicked(0))
					option = use;
			}
			else if (ImGui::BeginPopupContextItem("Edit Level")) {
				m_selected = index;
				setName();
				if (ImGui::MenuItem(m_openOrSave ? "Open" : "Save")) { option = use; }
				ImGui::Separator();
				if (ImGui::MenuItem("Delete")) { option = del; }
				ImGui::Separator();
				if (ImGui::MenuItem("Rename")) { option = rename; }
				ImGui::EndPopup();
			}
			ImGui::PopID();
			index++;
		}
		ImGui::EndChild();
		ImGui::Spacing();
		
		// Indicate which folder we're in
		const auto directory = "\\Maps\\" + m_subDirectory;
		ImGui::Text(directory.c_str());
		ImGui::SameLine();

		// Display an input field with a specific entry name
		const auto inputFlags = m_openOrSave ? ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_ReadOnly : ImGuiInputTextFlags_EnterReturnsTrue;
		if (ImGui::InputText(".bmap", nameInput, IM_ARRAYSIZE(nameInput), inputFlags))
			option = use;		

		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();
		ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(2.0f / 7.0f, 0.6f, 0.6f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(2.0f / 7.0f, 0.7f, 0.7f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(2.0f / 7.0f, 0.8f, 0.8f));
		if (ImGui::Button(m_openOrSave ? "Open" : "Save", { 50, 20 }))
			option = use;
		ImGui::SetItemDefaultFocus();
		ImGui::SameLine();
		ImGui::Spacing();
		ImGui::SameLine();
		ImGui::PopStyleColor(3);
		if (ImGui::Button("Cancel", { 75, 20 })) {
			m_popupOpen = false;
			ImGui::CloseCurrentPopup();
		}

		// Do something with the option chosen
		switch (option) {
		case use: {
			const auto& selectedLevel = m_levels[m_selected];
			if (selectedLevel.type == LevelEntry::back || selectedLevel.type == LevelEntry::folder) {
				const std::string nameCopy(selectedLevel.path);
				populateLevels(nameCopy);
				m_selected = -1;
			}
			else {
				if (m_openOrSave == true)
					m_editor->openLevel(selectedLevel.path);
				else 
					m_editor->saveLevel(std::string(nameInput) + ".bmap");
				m_popupOpen = false;
			}
			break;
		}
		case del:
			ImGui::OpenPopup("Delete Level");
			break;
		case rename:
			ImGui::OpenPopup("Rename Level");
			break;
		}

		// Draw 'Delete Level' confirmation
		bool openDelete = true;
		ImGui::SetNextWindowSize({ 350, 75 }, ImGuiCond_Appearing);
		if (ImGui::BeginPopupModal("Delete Level", &openDelete, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove)) {
			ImGui::TextWrapped("Are you sure you want to delete this level/folder, and all of its contents?\r\n");
			ImGui::Spacing();
			ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(0, 0.6f, 0.6f));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(0, 0.7f, 0.7f));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(0, 0.8f, 0.8f));
			if (ImGui::Button("Delete", { 50, 20 })) {
				const auto fullPath = std::filesystem::path(Engine::Get_Current_Dir() + "\\Maps\\" + m_levels[m_selected].path);
				std::filesystem::remove(fullPath);
				m_selected = -1;
				populateLevels(m_subDirectory);
				ImGui::CloseCurrentPopup();
			}
			ImGui::PopStyleColor(3);
			ImGui::SetItemDefaultFocus();
			ImGui::SameLine();
			ImGui::Spacing();
			ImGui::SameLine();
			if (ImGui::Button("Cancel", { 75, 20 }))
				ImGui::CloseCurrentPopup();
			ImGui::EndPopup();
		}

		// Draw 'Rename Level' dialogue
		bool openRename = true;
		if (ImGui::BeginPopupModal("Rename Level", &openRename, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove)) {
			ImGui::Text("Enter a new name for this level/folder...");
			ImGui::Spacing();
			char nameInput[256];
			for (size_t x = 0; x < m_levels[m_selected].name.length() && x < IM_ARRAYSIZE(nameInput); ++x)
				nameInput[x] = m_levels[m_selected].name[x];
			nameInput[std::min(256ull, m_levels[m_selected].name.length())] = '\0';
			if (ImGui::IsRootWindowOrAnyChildFocused() && !ImGui::IsAnyItemActive() && !ImGui::IsMouseClicked(0))
				ImGui::SetKeyboardFocusHere(0);
			if (ImGui::InputText("", nameInput, IM_ARRAYSIZE(nameInput), ImGuiInputTextFlags_EnterReturnsTrue)) {
				m_levels[m_selected].name = nameInput;
				if (m_levels[m_selected].type == LevelEntry::folder)
					m_levels[m_selected].name = "\\" + m_levels[m_selected].name + "\\";
				const auto fullPath = std::filesystem::path(Engine::Get_Current_Dir() + "\\Maps\\" + m_levels[m_selected].path);
				std::filesystem::rename(fullPath, std::filesystem::path(fullPath.parent_path().string() + "\\" + std::string(nameInput)));
				m_levels[m_selected].path = std::filesystem::path(fullPath.parent_path().string() + "\\" + std::string(nameInput)).string();
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}

		ImGui::EndPopup();
		freshlyOpened = false;
	}
	else
		freshlyOpened = true;
}

void LevelDialogue::startOpenDialogue()
{
	m_popupOpen = true;
	m_openOrSave = true;
}

void LevelDialogue::startSaveDialogue()
{
	m_popupOpen = true;
	m_openOrSave = false;
}

void LevelDialogue::populateLevels(const std::string& directory)
{
	m_levels.clear();
	m_subDirectory = directory;

	const auto rootPath = Engine::Get_Current_Dir() + "\\Maps\\";
	const auto path = std::filesystem::path(rootPath + directory);
	if (directory != "" && directory != "." && directory != "..")
		m_levels.push_back(LevelEntry{ "..", std::filesystem::relative(path.parent_path(), rootPath).string(), LevelEntry::back });
	for (auto& entry : std::filesystem::directory_iterator(path)) {
		LevelEntry prefabEntry{
			entry.path().filename().stem().string(),
			std::filesystem::relative(entry, rootPath).string()
		};
		if (entry.is_regular_file()) 
			prefabEntry.type = LevelEntry::file;		
		else if (entry.is_directory()) {
			prefabEntry.name = "\\" + prefabEntry.name + "\\";
			prefabEntry.type = LevelEntry::folder;
		}
		m_levels.push_back(prefabEntry);
	}
}