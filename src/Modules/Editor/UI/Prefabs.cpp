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
	// Load Texture Assets
	m_texBack = Shared_Texture(engine, "Editor//folderBack.png");
	m_texFolder = Shared_Texture(engine, "Editor//folder.png");
	m_texMissingThumb = Shared_Texture(engine, "Editor//prefab.png");

	// Load prefabs
	populatePrefabs();
}

void Prefabs::tick(const float& deltaTime)
{
	ImGui::SetNextWindowDockID(ImGui::GetID("LeftDock"), ImGuiCond_FirstUseEver);
	if (ImGui::Begin("Prefabs", NULL)) {
		static ImGuiTextFilter filter;
		ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 12.0f);
		filter.Draw("Search");
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
		for (int x = 0; x < m_prefabs.size(); ++x) {
			const auto& prefab = m_prefabs[x];
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
				ImGui::ImageButton((ImTextureID)static_cast<uintptr_t>(textureID), { 50, 50 }, { 0.0f, 1.0f }, { 1.0f, 0.0f });
				ImGui::TextWrapped(prefab.name.c_str());
				ImGui::EndGroup();
				if (ImGui::IsItemClicked() && ImGui::IsMouseDoubleClicked(0))
					selectPrefab(prefab);
				else if (ImGui::BeginPopupContextItem("Edit Prefab")) {
					if (ImGui::MenuItem("Open")) { selectPrefab(prefab); }
					ImGui::Separator();
					if (ImGui::MenuItem("Rename")) {  }
					ImGui::EndPopup();
				}
				ImGui::PopID();
				ImGui::NextColumn();
			}
		}
	}
	ImGui::End();

	if (m_selectedPrefab > -1)
		ImGui::OpenPopup("Name Prefab");
	if (ImGui::BeginPopupModal("Name Prefab", NULL, ImGuiWindowFlags_NoCollapse)) {
		ImGui::Text("Enter a name for this prefab");
		ImGui::Spacing();

		char nameInput[256];
		for (size_t x = 0; x < m_prefabs[m_selectedPrefab].name.length() && x < IM_ARRAYSIZE(nameInput); ++x)
			nameInput[x] = m_prefabs[m_selectedPrefab].name[x];
		nameInput[std::min(256ull, m_prefabs[m_selectedPrefab].name.length())] = '\0';
		if (ImGui::InputText("", nameInput, IM_ARRAYSIZE(nameInput)))
			m_prefabs[m_selectedPrefab].name = nameInput;
		ImGui::Spacing();

		if (ImGui::Button("Close")) {
			m_selectedPrefab = -1;
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
}

void Prefabs::makePrefab(const std::vector<ecsEntity*>& entities)
{
	// Changes To Make
	// Temporarily save this prefab data somewhere, probably to a variable
	// Then move the 'making prefab' work into the main tick
	// So we can create a proper naming window, checking for prefabs with existing name


	/*auto& world = m_engine->getModule_World();
	std::vector<char> prefabData;
	for each (const auto & entity in entities) {
		const auto entData = world.serializeEntity(entity);
		prefabData.insert(prefabData.end(), entData.begin(), entData.end());
	}
	m_prefabs.push_back({ "New Entity", prefabData });
	m_selectedPrefab = (int)(m_prefabs.size()) - 1;

	// Save Prefab to disk
	std::fstream mapFile(Engine::Get_Current_Dir() + "\\Maps\\Prefabs\\" + m_prefabSubDirectory + m_prefabs[m_selectedPrefab].name , std::ios::binary | std::ios::out);
	if (!mapFile.is_open())
		m_engine->getManager_Messages().error("Cannot write the binary map file to disk!");
	else
		mapFile.write(prefabData.data(), (std::streamsize)prefabData.size());
	mapFile.close();*/
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

void Prefabs::selectPrefab(const Prefab & prefab)
{
	if (prefab.type == Prefab::back || prefab.type == Prefab::folder) {
		const std::string nameCopy(prefab.path);
		populatePrefabs(nameCopy);
	}
	else {
		auto& world = m_engine->getModule_World();
		size_t dataRead(0ull);
		while (dataRead < prefab.serialData.size())
			world.deserializeEntity(prefab.serialData.data(), prefab.serialData.size(), dataRead);
	}
}