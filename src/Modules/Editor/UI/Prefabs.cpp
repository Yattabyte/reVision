#include "Modules/Editor/UI/Prefabs.h"
#include "Modules/Editor/Editor_M.h"
#include "Modules/World/ECS/components.h"
#include "Modules/World/World_M.h"
#include "Modules/UI/dear imgui/imgui.h"
#include "Engine.h"
#include <fstream>
#include <filesystem>


Prefabs::Prefabs(Engine* engine, LevelEditor_Module* editor)
	: m_engine(engine), m_editor(editor)
{
	// Load Assets
	m_texBack = Shared_Texture(engine, "Editor//folderBack.png");
	m_texFolder = Shared_Texture(engine, "Editor//folder.png");
	m_texMissingThumb = Shared_Texture(engine, "Editor//prefab.png");
	m_texIconRefresh = Shared_Texture(engine, "Editor//iconRefresh.png");

	// Load prefabs
	populatePrefabs();
}

void Prefabs::tick(const float& deltaTime)
{
	ImGui::SetNextWindowDockID(ImGui::GetID("LeftDock"), ImGuiCond_FirstUseEver);
	enum PrefabOptions {
		none,
		open,
		del,
		rename
	} prefabOption = none;
	// Draw Prefabs window
	if (ImGui::Begin("Prefabs", NULL)) {
		static ImGuiTextFilter filter;
		ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 12.0f);
		filter.Draw("Search");
		auto alignOffset = ImGui::GetWindowContentRegionMax().x - 19.0f;
		alignOffset = alignOffset < 0.0f ? 0.0f : alignOffset;
		ImGui::SameLine(alignOffset);
		if (ImGui::ImageButton((ImTextureID)static_cast<uintptr_t>(m_texIconRefresh->existsYet() ? m_texIconRefresh->m_glTexID : 0), { 15, 15 }, { 0.0f, 1.0f }, { 1.0f, 0.0f }))
			populatePrefabs(m_prefabSubDirectory);		
		ImGui::PopStyleVar();
		ImGui::Separator();
		ImGui::Spacing();
		const auto directory = "\\Prefabs\\" + m_prefabSubDirectory;
		ImGui::Text(directory.c_str());
		ImGui::Spacing();
		ImGuiStyle& style = ImGui::GetStyle();

		auto columnCount = int(float(ImGui::GetWindowContentRegionMax().x) / float((ImGui::GetStyle().ItemSpacing.x * 2) + 50));
		columnCount < 1 ? 1 : columnCount;
		ImGui::Columns(columnCount, nullptr, false);
		int count(0);
		for each (const auto& prefab in m_prefabs) {
			if (filter.PassFilter(prefab.name.c_str())) {
				ImGui::PushID(&prefab);
				GLuint textureID = 0;
				if (prefab.type == Prefab::back && m_texBack->existsYet())
					textureID = m_texBack->m_glTexID;
				else if (prefab.type == Prefab::folder && m_texFolder->existsYet())
					textureID = m_texFolder->m_glTexID;
				else if (prefab.type == Prefab::file && m_texMissingThumb->existsYet())
					textureID = m_texMissingThumb->m_glTexID;

				ImGui::BeginGroup();
				ImVec4 color = count == m_selectedIndex ? ImVec4(1,1,1,0.75) : ImVec4(0,0,0,0);
				ImGui::ImageButton(
					(ImTextureID)static_cast<uintptr_t>(textureID), 
					{ 50, 50 }, 
					{ 0.0f, 1.0f }, { 1.0f, 0.0f }, 
					-1, 
					color
				);
				ImGui::TextWrapped(prefab.name.c_str());
				ImGui::EndGroup();
				if (ImGui::IsItemClicked()) {
					m_selectedIndex = count;
					if (ImGui::IsMouseDoubleClicked(0))
						prefabOption = open;
				}
				else if (ImGui::BeginPopupContextItem("Edit Prefab")) {
					m_selectedIndex = count;
					if (ImGui::MenuItem("Open")) { prefabOption = open; }
					ImGui::Separator();
					if (ImGui::MenuItem("Delete")) { prefabOption = del; }
					ImGui::Separator();
					if (ImGui::MenuItem("Rename")) { prefabOption = rename; }
					ImGui::EndPopup();
				}
				ImGui::PopID();
				ImGui::NextColumn();
			}
			count++;
		}
	}
	ImGui::End();

	// Do something with the option chosen
	bool openDelete = true, openRename = true;
	switch (prefabOption) {
	case open:
		openPrefab();
		break;
	case del:
		ImGui::OpenPopup("Delete Prefab");
		break;
	case rename:
		ImGui::OpenPopup("Rename Prefab");
		break;
	}
	
	// Draw 'Delete Prefab' confirmation
	ImGui::SetNextWindowSize({ 350, 90 }, ImGuiCond_Appearing);
	if (ImGui::BeginPopupModal("Delete Prefab", &openDelete, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove)) {
		ImGui::TextWrapped(
			"Are you sure you want to delete this prefab?\n"
			"This won't remove the prefab from any levels."
		);
		ImGui::Spacing();
		ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(0, 0.6f, 0.6f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(0, 0.7f, 0.7f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(0, 0.8f, 0.8f));
		if (ImGui::Button("Delete", { 50, 20 })) {
			const auto fullPath = std::filesystem::path(Engine::Get_Current_Dir() + "\\Maps\\Prefabs\\" + m_prefabs[m_selectedIndex].path);
			std::filesystem::remove(fullPath);
			m_selectedIndex = -1;
			populatePrefabs(m_prefabSubDirectory);
			ImGui::CloseCurrentPopup();
		}
		ImGui::SetItemDefaultFocus();
		ImGui::SameLine();
		ImGui::Spacing();
		ImGui::SameLine();
		ImGui::PopStyleColor(3);
		if (ImGui::Button("Cancel", { 75, 20 }))
			ImGui::CloseCurrentPopup();
		ImGui::EndPopup();
	}

	// Draw 'Rename Prefab' dialog
	if (ImGui::BeginPopupModal("Rename Prefab", &openRename, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove)) {
		ImGui::Text("Enter a new name for this prefab...");
		ImGui::Spacing();

		char nameInput[256];
		for (size_t x = 0; x < m_prefabs[m_selectedIndex].name.length() && x < IM_ARRAYSIZE(nameInput); ++x)
			nameInput[x] = m_prefabs[m_selectedIndex].name[x];
		nameInput[std::min(256ull, m_prefabs[m_selectedIndex].name.length())] = '\0';
		if (ImGui::IsRootWindowOrAnyChildFocused() && !ImGui::IsAnyItemActive() && !ImGui::IsMouseClicked(0))
			ImGui::SetKeyboardFocusHere(0);
		if (ImGui::InputText("", nameInput, IM_ARRAYSIZE(nameInput), ImGuiInputTextFlags_EnterReturnsTrue)) {
			m_prefabs[m_selectedIndex].name = nameInput;
			const auto fullPath = std::filesystem::path(Engine::Get_Current_Dir() + "\\Maps\\Prefabs\\" + m_prefabs[m_selectedIndex].path);
			std::filesystem::rename(fullPath, std::filesystem::path(fullPath.parent_path().string() + "\\" + std::string(nameInput)));
			m_prefabs[m_selectedIndex].path = std::filesystem::path(fullPath.parent_path().string() + "\\" + std::string(nameInput)).string();
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
}

void Prefabs::makePrefab(const std::vector<ecsEntity*>& entities)
{
	auto& world = m_engine->getModule_World();
	std::vector<char> prefabData;
	for each (const auto & entity in entities) {
		const auto entData = world.serializeEntity(entity);
		prefabData.insert(prefabData.end(), entData.begin(), entData.end());
	}
	m_prefabs.push_back({ "New Entity", m_prefabSubDirectory + "\\New Entity", Prefab::file, prefabData });
	m_selectedIndex = (int)(m_prefabs.size()) - 1;

	// Save Prefab to disk
	std::fstream mapFile(Engine::Get_Current_Dir() + "\\Maps\\Prefabs\\" + m_prefabs[m_selectedIndex].path , std::ios::binary | std::ios::out);
	if (!mapFile.is_open())
		m_engine->getManager_Messages().error("Cannot write the binary map file to disk!");
	else
		mapFile.write(prefabData.data(), (std::streamsize)prefabData.size());
	mapFile.close();
}

void Prefabs::populatePrefabs(const std::string & directory)
{
	m_prefabs.clear();
	m_prefabSubDirectory = directory;

	const auto rootPath = Engine::Get_Current_Dir() + "\\Maps\\Prefabs\\";
	const auto path = std::filesystem::path(rootPath + directory);
	if (directory != "" && directory != "." && directory != "..")
		m_prefabs.push_back(Prefab{ "back", std::filesystem::relative(path.parent_path(), rootPath).string(), Prefab::back, {} });
	for (auto& entry : std::filesystem::directory_iterator(path)) {
		Prefab prefabEntry{
			entry.path().filename().string(),
			std::filesystem::relative(entry, rootPath).string()
		};
		if (entry.is_regular_file()) {
			prefabEntry.type = Prefab::file;
			std::fstream prefabFile(entry, std::ios::binary | std::ios::beg || std::ios::in);
			if (prefabFile.is_open()) {
				prefabEntry.serialData.resize(std::filesystem::file_size(entry));
				prefabFile.read(&prefabEntry.serialData[0], (std::streamsize)prefabEntry.serialData.size());
			}
			prefabFile.close();
		}
		else if (entry.is_directory())
			prefabEntry.type = Prefab::folder;
		m_prefabs.push_back(prefabEntry);
	}
}

void Prefabs::openPrefab()
{
	const auto& selectedPrefab = m_prefabs[m_selectedIndex];
	if (selectedPrefab.type == Prefab::back || selectedPrefab.type == Prefab::folder) {
		const std::string nameCopy(selectedPrefab.path);
		populatePrefabs(nameCopy);
		m_selectedIndex = -1;
	}
	else {
		auto& world = m_engine->getModule_World();
		size_t dataRead(0ull);
		glm::vec3 center(0.0f);
		std::vector<Transform_Component*> transformComponents;
		while (dataRead < selectedPrefab.serialData.size()) {
			if (auto * entity = world.deserializeEntity(selectedPrefab.serialData.data(), selectedPrefab.serialData.size(), dataRead))
				if (auto * transform = world.getComponent<Transform_Component>(entity)) {
					transformComponents.push_back(transform);
					center += transform->m_localTransform.m_position;
				}
		}

		// Treat entity collection as a group
		// Move the group to world origin, then transform to 3D cursor
		center /= transformComponents.size();
		const auto cursorPos = m_editor->getGizmoPosition();
		for each (auto * transform in transformComponents) {
			transform->m_localTransform.m_position = (transform->m_localTransform.m_position - center) + cursorPos;
			transform->m_localTransform.update();
		}
	}
}