#include "Modules/Editor/UI/OpenDialogue.h"
#include "Modules/Editor/Editor_M.h"
#include "Engine.h"
#include "imgui.h"
#include "imgui_internal.h"
#include <filesystem>
#include <sstream>
#include <time.h>


OpenDialogue::OpenDialogue(Engine& engine, LevelEditor_Module& editor) :
	m_engine(engine),
	m_editor(editor),
	m_iconFile(Shared_Texture(engine, "Editor//iconFile.png")),
	m_iconFolder(Shared_Texture(engine, "Editor//iconFolder.png")),
	m_iconBack(Shared_Texture(engine, "Editor//iconBack.png")),
	m_iconRefresh(Shared_Texture(engine, "Editor//iconRefresh.png"))
{
	m_open = false;
}

void OpenDialogue::tick(const float& /*deltaTime*/)
{
	tickMainDialogue();
}

void OpenDialogue::populateLevels(const std::string& directory)
{
	m_levels.clear();
	m_subDirectory = directory;

	const auto rootPath = Engine::Get_Current_Dir() + "\\Maps\\";
	const auto path = std::filesystem::path(rootPath + directory);
	if (!directory.empty() && directory != "." && directory != "..")
		m_levels.push_back(LevelEntry{ "..", std::filesystem::relative(path.parent_path(), rootPath).string(), "", "", "", "", LevelEntry::Type::BACK });
	for (auto& entry : std::filesystem::directory_iterator(path)) {
		std::string timeString;
		struct _stat64 fileInfo{};
		if (_wstati64(entry.path().wstring().c_str(), &fileInfo) == 0) {
			const auto* const t = std::localtime(&fileInfo.st_mtime);
			timeString = std::to_string(t->tm_hour > 12 ? 24 - t->tm_hour : t->tm_hour) + ":" + std::to_string(t->tm_min)
				+ (t->tm_hour >= 12 ? "PM " : "AM ") + std::to_string(t->tm_mday) + "/" + std::to_string(t->tm_mon) + "/" + std::to_string(t->tm_year - 100);
		}
		constexpr static auto readableFileSize = [](const size_t& size) -> std::string {
			auto remainingSize = static_cast<double>(size);
			constexpr static char* units[] = { " B", " KB", " MB", " GB", " TB", " PB", " EB" };
			int i = 0;
			while (remainingSize > 1024.00) {
				remainingSize /= 1024.00;
				++i;
			}
			std::stringstream stream;
			stream << std::fixed << std::setprecision(2) << remainingSize;
			return stream.str() + units[i];
		};
		LevelEntry prefabEntry{
			entry.path().filename().stem().string(),
			std::filesystem::relative(entry, rootPath).string(),
			entry.path().extension().string(),
			"File",
			timeString,
			readableFileSize(entry.file_size())
		};
		if (entry.is_regular_file()) {
			prefabEntry.type = LevelEntry::Type::FILE;
			if (prefabEntry.extension == ".bmap")
				prefabEntry.extType = "Map";
			else if (prefabEntry.extension == ".autosave")
				prefabEntry.extType = "Autosave";
		}
		else if (entry.is_directory()) {
			prefabEntry.type = LevelEntry::Type::FOLDER;
			prefabEntry.extType = "Folder";
		}
		m_levels.push_back(prefabEntry);
	}
}

void OpenDialogue::tickMainDialogue()
{
	static bool freshlyOpened = true; // flag used for operations that should happen only once-per-opening
	const auto title = "Open Level";
	if (m_open)
		ImGui::OpenPopup(title);
	ImGui::SetNextWindowSize({ 600, 500 }, ImGuiCond_Appearing);
	if (ImGui::BeginPopupModal(title, &m_open, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove)) {
		enum class DialogueOptions {
			NONE,
			USE,
			CLONE,
			RENAME,
			DELETE,
		} option = DialogueOptions::NONE;
		if (freshlyOpened)
			populateLevels();

		// Header
		ImGui::Text("Choose a level to open...");
		ImGui::SameLine(std::max(ImGui::GetWindowContentRegionMax().x - 28.0F, 0.0F));
		if (ImGui::ImageButton((ImTextureID)static_cast<uintptr_t>(m_iconRefresh->ready() ? m_iconRefresh->m_glTexID : 0), { 15, 15 }, { 0.0F, 1.0F }, { 1.0F, 0.0F }))
			populateLevels(m_subDirectory);
		ImGui::Spacing();

		// Display a list of level entries for the directory chosen
		ImGui::BeginChild("Level List", ImVec2(580, ImGui::GetWindowContentRegionMax().y - 85), true);
		ImGui::Text("Name");
		ImGui::SameLine(250);
		ImGui::Text("Type");
		ImGui::SameLine(350);
		ImGui::Text("Date Modified");
		ImGui::SameLine(475);
		ImGui::Text("Size");
		ImGui::Separator();
		int index = 0;
		for (const auto& level : m_levels) {
			GLuint icon = (level.type == LevelEntry::Type::FILE && m_iconFile->ready()) ? m_iconFile->m_glTexID :
				(level.type == LevelEntry::Type::FOLDER && m_iconFolder->ready()) ? m_iconFolder->m_glTexID :
				(level.type == LevelEntry::Type::BACK && m_iconBack->ready()) ? m_iconBack->m_glTexID : 0;
			ImGui::PushID(index);
			ImGui::Image((ImTextureID)static_cast<uintptr_t>(icon), ImVec2(15, 15), { 0.0F, 1.0F }, { 1.0F, 0.0F });
			ImGui::SameLine(0);
			const auto name = level.name + level.extension;
			ImGui::Selectable(name.c_str(), m_selected == index);
			if (ImGui::IsItemClicked()) {
				m_selected = index;
				if (ImGui::IsMouseDoubleClicked(0))
					option = DialogueOptions::USE;
			}
			else if (ImGui::BeginPopupContextItem("Edit Level")) {
				m_selected = index;
				if (ImGui::MenuItem("Open")) { option = DialogueOptions::USE; }
				ImGui::Separator();
				if (ImGui::MenuItem("Clone")) { option = DialogueOptions::CLONE; }
				if (ImGui::MenuItem("Rename")) { option = DialogueOptions::RENAME; }
				ImGui::Separator();
				if (ImGui::MenuItem("Delete")) { option = DialogueOptions::DELETE; }
				ImGui::EndPopup();
			}
			ImGui::SameLine(250);
			ImGui::Text("%s", level.extType.c_str());
			ImGui::SameLine(350);
			ImGui::Text("%s", level.date.c_str());
			ImGui::SameLine(475);
			ImGui::Text("%s", level.size.c_str());
			ImGui::PopID();
			index++;
		}
		ImGui::EndChild();
		ImGui::Spacing();

		// Display an open button
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(ImColor::HSV(2.0F / 7.0F, 0.6F, 0.6F)));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(ImColor::HSV(2.0F / 7.0F, 0.7F, 0.7F)));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(ImColor::HSV(2.0F / 7.0F, 0.8F, 0.8F)));
		ImGui::PushItemFlag(ImGuiItemFlags_Disabled, m_selected == -1);
		ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * ((m_selected == -1) ? 0.25F : 1.0F));
		if (ImGui::Button("Open", { 75, 20 }))
			option = DialogueOptions::USE;
		ImGui::PopItemFlag();
		ImGui::PopStyleVar();
		ImGui::PopStyleColor(3);
		ImGui::SameLine();
		ImGui::Spacing();
		ImGui::SameLine();

		// Display a cancel button
		if (ImGui::Button("Cancel", { 100, 20 })) {
			m_open = false;
			ImGui::CloseCurrentPopup();
		}
		ImGui::SetItemDefaultFocus();

		if (option == DialogueOptions::USE) {
			const auto& selectedLevel = m_levels[m_selected];
			if (selectedLevel.type == LevelEntry::Type::BACK || selectedLevel.type == LevelEntry::Type::FOLDER) {
				const std::string nameCopy(selectedLevel.path);
				populateLevels(nameCopy);
				m_selected = -1;
			}
			else {
				m_editor.openLevel(selectedLevel.path);
				m_selected = -1;
				m_open = false;
			}
		}

		if (option == DialogueOptions::CLONE) {
			const auto srcPath = std::filesystem::path(Engine::Get_Current_Dir() + "\\Maps\\" + m_levels[m_selected].path);
			auto dstPath = srcPath;
			int count = 1;
			while (std::filesystem::exists(dstPath))
				dstPath.replace_filename(
				(srcPath.has_stem() ? srcPath.stem().string() : srcPath.filename().string())
					+ " - Copy (" + std::to_string(count++) + ")"
					+ (srcPath.has_extension() ? srcPath.extension().string() : "")
				);
			std::filesystem::copy(srcPath, dstPath);
			populateLevels(m_subDirectory);
		}

		if (option == DialogueOptions::RENAME)
			ImGui::OpenPopup("Rename Level");
		tickRenameDialogue();

		if (option == DialogueOptions::DELETE)
			ImGui::OpenPopup("Delete Level");
		tickDeleteDialogue();


		ImGui::EndPopup();
		freshlyOpened = false;
	}
	else
		freshlyOpened = true;
}

void OpenDialogue::tickRenameDialogue()
{
	bool openRename = true;
	if (ImGui::BeginPopupModal("Rename Level", &openRename, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove)) {
		ImGui::Text("Enter a new name for this item...");
		ImGui::Spacing();
		char nameInput[256]{};
		const auto nameLength = m_levels[m_selected].name.length();
		for (size_t x = 0; x < nameLength && x < IM_ARRAYSIZE(nameInput); ++x)
			nameInput[x] = m_levels[m_selected].name[x];
		nameInput[std::min(256ULL, m_levels[m_selected].name.length())] = '\0';
		if (ImGui::IsAnyWindowFocused() && !ImGui::IsAnyItemActive() && !ImGui::IsMouseClicked(0))
			ImGui::SetKeyboardFocusHere(0);
		if (ImGui::InputText("", nameInput, IM_ARRAYSIZE(nameInput), ImGuiInputTextFlags_EnterReturnsTrue)) {
			m_levels[m_selected].name = nameInput;
			if (m_levels[m_selected].type == LevelEntry::Type::FOLDER)
				m_levels[m_selected].name = "\\" + m_levels[m_selected].name + "\\";
			const auto oldPath = std::filesystem::path(Engine::Get_Current_Dir() + "\\Maps\\" + m_levels[m_selected].path);
			const auto newPath = std::filesystem::path(oldPath.parent_path().string() + "\\" + std::string(nameInput) + (oldPath.has_extension() ? oldPath.extension().string() : "")).string();
			std::filesystem::rename(oldPath, newPath);
			m_levels[m_selected].path = std::filesystem::relative(newPath, Engine::Get_Current_Dir() + "\\Maps\\").string();
			m_levels[m_selected].name = std::string(nameInput);
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
}

void OpenDialogue::tickDeleteDialogue()
{
	bool openDelete = true;
	ImGui::SetNextWindowSize({ 350, 95 }, ImGuiCond_Appearing);
	if (ImGui::BeginPopupModal("Delete Level", &openDelete, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove)) {
		ImGui::TextWrapped("Are you sure you want to delete this item?\r\nThis action is irreversible.\r\n");
		ImGui::Spacing(); ImGui::Spacing();
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(ImColor::HSV(0, 0.6F, 0.6F)));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(ImColor::HSV(0, 0.7F, 0.7F)));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(ImColor::HSV(0, 0.8F, 0.8F)));
		if (ImGui::Button("Delete", { 75, 20 })) {
			const auto fullPath = std::filesystem::path(Engine::Get_Current_Dir() + "\\Maps\\" + m_levels[m_selected].path);
			std::error_code ec;
			if (std::filesystem::remove_all(fullPath, ec) != 0U)
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