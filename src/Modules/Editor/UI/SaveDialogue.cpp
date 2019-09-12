#include "Modules/Editor/UI/SaveDialogue.h"
#include "Modules/Editor/Editor_M.h"
#include "Modules/UI/dear imgui/imgui.h"
#include "Engine.h"
#include <filesystem>


SaveDialogue::SaveDialogue(Engine* engine, LevelEditor_Module* editor)
	: m_engine(engine), m_editor(editor)
{
}

void SaveDialogue::tick(const float& deltaTime)
{
	tickMainDialogue();
}

void SaveDialogue::startDialogue()
{
	m_popupOpen = true;
}

void SaveDialogue::populateLevels(const std::string& directory)
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

void SaveDialogue::tickMainDialogue()
{
	static bool freshlyOpened = true; // flag used for operations that should happen only once-per-opening
	const auto title = "Save Level";
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

		// Header
		ImGui::Text("Choose a name to save the level as...");
		ImGui::Spacing();

		// Display a list of level entries for the directory chosen
		int index = 0;
		ImGui::BeginChild("Level List", ImVec2(580, ImGui::GetWindowContentRegionMax().y - 105), true);
		for each (const auto& level in m_levels) {
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
				if (ImGui::MenuItem("Save")) { option = use; }
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

		// Display an input field with a specific entry name on the same line
		const auto inputFlags = ImGuiInputTextFlags_EnterReturnsTrue;
		if (ImGui::InputText(".bmap", nameInput, IM_ARRAYSIZE(nameInput), inputFlags))
			option = use;
		ImGui::Spacing(); ImGui::Spacing();

		// Display a save button
		ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(2.0f / 7.0f, 0.6f, 0.6f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(2.0f / 7.0f, 0.7f, 0.7f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(2.0f / 7.0f, 0.8f, 0.8f));
		if (ImGui::Button("Save", { 75, 20 }))
			option = use;
		ImGui::PopStyleColor(3);
		ImGui::SameLine();
		ImGui::Spacing();
		ImGui::SameLine();

		// Display a cancel button
		if (ImGui::Button("Cancel", { 100, 20 })) {
			m_popupOpen = false;
			ImGui::CloseCurrentPopup();
		}
		ImGui::SetItemDefaultFocus();

		if (option == use) {
			const auto& selectedLevel = m_levels[m_selected];
			if (selectedLevel.type == LevelEntry::back || selectedLevel.type == LevelEntry::folder) {
				const std::string nameCopy(selectedLevel.path);
				populateLevels(nameCopy);
				m_selected = -1;
			}
			else {
				constexpr const auto compareNCase = [](const std::string& str1, const std::string& str2) {
					return ((str1.size() == str2.size()) && std::equal(str1.begin(), str1.end(), str2.begin(), [](const char& c1, const char& c2) {
						return (c1 == c2 || std::toupper(c1) == std::toupper(c2));
						}));
				};
				bool alreadyExists = false;
				m_chosen = std::string(nameInput);
				for each (const auto& level in m_levels)
					if (compareNCase(level.name, m_chosen) && !compareNCase(level.name + ".bmap", m_editor->getMapName())) {
						alreadyExists = true;
						break;
					}
				if (alreadyExists)
					ImGui::OpenPopup("Overwrite Level");
				else {
					m_editor->saveLevel(m_chosen + ".bmap");
					m_popupOpen = false;
				}
			}
		}
		tickOverwriteDialogue();

		if (option == del)
			ImGui::OpenPopup("Delete Level");
		tickDeleteDialogue();

		if (option == rename)
			ImGui::OpenPopup("Rename Level");
		tickRenameDialogue();

		ImGui::EndPopup();
		freshlyOpened = false;
	}
	else
		freshlyOpened = true;
}

void SaveDialogue::tickOverwriteDialogue()
{
	bool openOverwrite = true;
	ImGui::SetNextWindowSize({ 400, 106 }, ImGuiCond_Appearing);
	if (ImGui::BeginPopupModal("Overwrite Level", &openOverwrite, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove)) {
		ImGui::TextWrapped("This is a different file, are you sure you want to overwrite it?\r\nThis action is irreversable.\r\n");
		ImGui::Spacing(); ImGui::Spacing();
		ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(2.0f / 7.0f, 0.6f, 0.6f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(2.0f / 7.0f, 0.7f, 0.7f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(2.0f / 7.0f, 0.8f, 0.8f));
		if (ImGui::Button("Overwrite", { 75, 20 })) {
			m_editor->saveLevel(m_chosen + ".bmap");
			ImGui::CloseCurrentPopup();
			m_popupOpen = false;
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

void SaveDialogue::tickDeleteDialogue()
{
	bool openDelete = true;
	ImGui::SetNextWindowSize({ 350, 95 }, ImGuiCond_Appearing);
	if (ImGui::BeginPopupModal("Delete Level", &openDelete, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove)) {
		ImGui::TextWrapped("Are you sure you want to delete this item?\r\nThis action is irreversable.\r\n");
		ImGui::Spacing(); ImGui::Spacing();
		ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(0, 0.6f, 0.6f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(0, 0.7f, 0.7f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(0, 0.8f, 0.8f));
		if (ImGui::Button("Delete", { 75, 20 })) {
			const auto fullPath = std::filesystem::path(Engine::Get_Current_Dir() + "\\Maps\\" + m_levels[m_selected].path);
			std::filesystem::remove(fullPath);
			m_selected = -1;
			populateLevels(m_subDirectory);
			ImGui::CloseCurrentPopup();
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

void SaveDialogue::tickRenameDialogue()
{
	bool openRename = true;
	if (ImGui::BeginPopupModal("Rename Level", &openRename, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove)) {
		ImGui::Text("Enter a new name for this item...");
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
}