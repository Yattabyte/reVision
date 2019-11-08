#include "Modules/Editor/UI/Prefabs.h"
#include "Modules/Editor/Editor_M.h"
#include "Modules/Graphics/Common/Camera.h"
#include "Modules/Graphics/Common/Viewport.h"
#include "Modules/Graphics/Graphics_M.h"
#include "Modules/ECS/component_types.h"
#include "imgui.h"
#include "Engine.h"
#include <fstream>
#include <filesystem>


Prefabs::~Prefabs()
{
	// Update indicator
	*m_aliveIndicator = false;
}

Prefabs::Prefabs(Engine* engine, LevelEditor_Module* editor)
	: m_engine(engine), m_editor(editor)
{
	m_open = true;
	m_viewport = std::make_shared<Viewport>(glm::vec2(0.0f), glm::vec2((float)m_thumbSize), engine);

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
	if (m_open) {
		tickThumbnails(deltaTime);
		tickWindow(deltaTime);
		tickPopupDialogues(deltaTime);
	}
}

void Prefabs::addPrefab(Prefabs::Entry& prefab)
{
	glm::vec3 center(0.0f);
	std::vector<Transform_Component*> transformComponents;
	for each (const auto & entityHandle in prefab.entityHandles) {
		if (auto* transform = m_previewWorld.getComponent<Transform_Component>(entityHandle)) {
			transformComponents.push_back(transform);
			center += transform->m_localTransform.m_position;
		}
	}

	// Move the entities in this prefab by 500 units on each axis
	const auto cursorPos = glm::vec3(m_prefabs.size() * 1000.0f * glm::vec3(0, -1, 0));
	if (transformComponents.size()) {
		center /= transformComponents.size();
		for each (auto * transform in transformComponents) {
			transform->m_localTransform.m_position = (transform->m_localTransform.m_position - center) + cursorPos;
			transform->m_localTransform.update();
		}
	}
	prefab.spawnPoint = cursorPos;

	// Create the camera and move it to where the entity is located
	auto camera = std::make_shared<Camera>();
	(*camera)->Dimensions = glm::vec2((float)m_thumbSize);
	(*camera)->FarPlane = 250.0f;
	(*camera)->FOV = 90.0F;
	(*camera)->vMatrix = glm::translate(glm::mat4(1.0f), cursorPos);
	(*camera)->vMatrixInverse = glm::inverse((*camera)->vMatrix);
	(*camera)->EyePosition = glm::vec3(0, 0, -25.0f);
	const float verticalRad = 2.0f * atanf(tanf(glm::radians(90.0f) / 2.0f) / 1.0F);
	(*camera)->pMatrix = glm::perspective(verticalRad, 1.0f, Camera::ConstNearPlane, (*camera)->FarPlane);
	(*camera)->pMatrixInverse = glm::inverse((*camera)->pMatrix);
	(*camera)->pvMatrix = (*camera)->pMatrix * (*camera)->vMatrix;
	m_prefabCameras.push_back(camera);

	// Create the thumbnail texture
	glCreateTextures(GL_TEXTURE_2D, 1, &prefab.texID);
	glTextureStorage2D(prefab.texID, 1, GL_RGB16F, m_thumbSize, m_thumbSize);
	glTextureParameteri(prefab.texID, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTextureParameteri(prefab.texID, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	m_prefabs.emplace_back(prefab);
};

void Prefabs::addPrefab(const std::vector<char>& entityData)
{
	Prefabs::Entry newPrefab = { "New Entity", m_prefabSubDirectory + "\\New Entity", Entry::file };

	size_t dataRead(0ull);
	while (dataRead < entityData.size())
		newPrefab.entityHandles.push_back(
			std::get<0>(m_previewWorld.deserializeEntity(&entityData[0], entityData.size(), dataRead))
		);
	addPrefab(newPrefab);
	m_selectedIndex = (int)(m_prefabs.size()) - 1;

	// Save Prefab to disk
	std::ofstream mapFile(Engine::Get_Current_Dir() + "\\Maps\\Prefabs\\" + m_prefabs[m_selectedIndex].path, std::ios::binary | std::ios::out);
	if (!mapFile.is_open())
		m_engine->getManager_Messages().error("Cannot write the binary map file to disk!");
	else
		mapFile.write(entityData.data(), (std::streamsize)entityData.size());
	mapFile.close();
}

void Prefabs::populatePrefabs(const std::string& directory)
{
	// Delete the textures
	for each (auto & entry in m_prefabs)
		glDeleteTextures(1, &entry.texID);

	// Reset prefab data
	m_prefabSubDirectory = directory;
	m_prefabs.clear();
	m_prefabCameras.clear();
	m_previewWorld.clear();

	const auto rootPath = Engine::Get_Current_Dir() + "\\Maps\\Prefabs\\";
	const auto path = std::filesystem::path(rootPath + directory);

	// Add an entry to go back a folder
	if (directory != "" && directory != "." && directory != "..")
		m_prefabs.emplace_back(Entry{ "back", std::filesystem::relative(path.parent_path(), rootPath).string(), Entry::back, {} });


	// If in the root folder, add hard-coded prefab entries
	if (directory == "" || directory == ".") {
		// Basic Model Prefab
		{
			Transform_Component a;
			BoundingBox_Component b;
			Prop_Component c;
			a.m_localTransform.m_scale = glm::vec3(15.0f);
			a.m_localTransform.update();
			c.m_modelName = "FireHydrant\\FireHydrantMesh.obj";
			ecsBaseComponent* entityComponents[] = { &a, &b, &c };
			Prefabs::Entry entry{ "Basic Model", "", Entry::file, {m_previewWorld.makeEntity(entityComponents, 3ull, "Basic Model")} };
			addPrefab(entry);
		}
		/*// Basic Sun Prefab
		{
			Transform_Component a;
			LightDirectional_Component b;
			Shadow_Component c;
			CameraArray_Component d;
			LightColor_Component e;
			a.m_localTransform.m_orientation = glm::quat(0.707, 0, 0, -0.707);
			a.m_localTransform.update();
			e.m_color = glm::vec3(1.0f);
			e.m_intensity = 15.0f;
			ecsBaseComponent* entityComponents[] = { &a, &b, &c, &d, &e };
			addPrefab(Prefabs::Entry{ "Basic Sun", "", Entry::file, m_previewWorld.makeEntity(entityComponents, 5ull, "Basic Sun") });
		}*/
		// Basic Light
		/*{
			Transform_Component a;
			Light_Component b;
			Shadow_Component c;
			a.m_localTransform.m_orientation = glm::quat(0.653, -0.271, 0.653, -0.271);
			a.m_localTransform.update();
			b.m_type = Light_Component::Light_Type::DIRECTIONAL;
			b.m_color = glm::vec3(1.0f);
			b.m_intensity = 15.0f;
			b.m_radius = 1.0F;
			b.m_cutoff = 180.0f;
			ecsBaseComponent* entityComponents[] = { &a, &b, &c };
			addPrefab(Prefabs::Entry{ "Basic Sun", "", Entry::file, {m_previewWorld.makeEntity(entityComponents, 3ull, "Basic Sun")} });
		}*/
	}

	// Cycle through each entry on disk, making prefab entries
	for (auto& entry : std::filesystem::directory_iterator(path)) {
		Prefabs::Entry newPrefab{ entry.path().filename().string(), std::filesystem::relative(entry, rootPath).string() };
		if (entry.is_regular_file()) {
			newPrefab.type = Entry::file;
			std::ifstream prefabFile(entry, std::ios::beg);
			if (prefabFile.is_open()) {
				// To Do: Remove serial data from entry class
				const auto size = std::filesystem::file_size(entry);
				std::vector<char> data(size);
				prefabFile.read(&data[0], (std::streamsize)size);
				size_t dataRead(0ull);
				while (dataRead < data.size())
					newPrefab.entityHandles.push_back(
						std::get<0>(m_previewWorld.deserializeEntity(&data[0], size, dataRead))
					);
			}
			prefabFile.close();
		}
		else if (entry.is_directory())
			newPrefab.type = Entry::folder;
		addPrefab(newPrefab);
	}

	// Add sun tilted 45 deg to preview world to show off prefabs
	{
		Transform_Component a;
		Light_Component b;
		b.m_type = Light_Component::Light_Type::DIRECTIONAL;
		b.m_color = glm::vec3(1.0, 0.9, 0.8);
		b.m_intensity = 10.0f;
		b.m_radius = 1000.0f;
		b.m_cutoff = 180.0f;
		ecsBaseComponent* entityComponents[] = { &a, &b };
		m_sunHandle = m_previewWorld.makeEntity(entityComponents, 2ull, "Preview Sun");
	}
}

void Prefabs::openPrefabEntry()
{
	const auto& selectedPrefab = m_prefabs[m_selectedIndex];
	if (selectedPrefab.type == Entry::back || selectedPrefab.type == Entry::folder) {
		const std::string nameCopy(selectedPrefab.path);
		populatePrefabs(nameCopy);
		m_selectedIndex = -1;
	}
	else
		for each (const auto & handle in selectedPrefab.entityHandles)
			m_editor->addEntity(m_previewWorld.serializeEntity(handle));
}

void Prefabs::tickThumbnails(const float& deltaTime)
{
	auto rotation = glm::quat_cast(m_engine->getModule_Graphics().getClientCamera()->get()->vMatrix);
	const auto rotMat = glm::mat4_cast(rotation);
	auto& trans = *m_previewWorld.getComponent<Transform_Component>(m_sunHandle);
	trans.m_localTransform.m_orientation = glm::inverse(glm::rotate(glm::quat(1, 0, 0, 0), glm::radians(45.0f), glm::vec3(1, 0, 0)) * rotation);
	trans.m_localTransform.update();

	int count(0);
	for each (auto & prefab in m_prefabs) {
		glm::vec3 minExtents(FLT_MAX), maxExtents(FLT_MIN);
		for each (const auto & entityHandle in prefab.entityHandles) {
			glm::vec3 scale(1.0f);
			if (auto* trans = m_previewWorld.getComponent<Transform_Component>(entityHandle))
				scale = trans->m_worldTransform.m_scale;
			if (auto* prop = m_previewWorld.getComponent<Prop_Component>(entityHandle))
				if (const auto& model = prop->m_model; model->existsYet()) {
					minExtents = glm::min(minExtents, model->m_bboxMin * scale);
					maxExtents = glm::max(maxExtents, model->m_bboxMax * scale);
				}
		}
		const auto extents = (maxExtents - minExtents) / 2.0f, center = ((maxExtents - minExtents) / 2.0f) + minExtents;

		auto& camera = m_prefabCameras[count];
		const auto objectSize = glm::compMax(extents);
		const auto cameraView = 2.0f * tanf(0.5f * glm::radians(90.0f)); // Visible height 1 meter in front
		auto distance = 3.0f * objectSize / cameraView; // Combined wanted distance from the object
		distance += 0.5f * objectSize; // Estimated offset from the center to the outside of the object
		// Rotate distance based on editor camera rotation
		const auto rotatedPosition = glm::inverse(rotMat) * glm::vec4(0, 0, distance, 1);
		const auto newPosition = prefab.spawnPoint + center + glm::vec3(rotatedPosition);
		(*camera)->EyePosition = newPosition;
		(*camera)->vMatrix = rotMat * glm::translate(glm::mat4(1.0f), -glm::vec3(newPosition));
		(*camera)->vMatrixInverse = glm::inverse((*camera)->vMatrix);
		(*camera)->pvMatrix = (*camera)->pMatrix * (*camera)->vMatrix;
		count++;
	}

	GLint previousFBO(0);
	glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &previousFBO);
	m_viewport->resize(glm::vec2((float)m_thumbSize), (int)m_prefabs.size());
	m_engine->getModule_Graphics().renderWorld(m_previewWorld, deltaTime, m_viewport, m_prefabCameras);
	const auto screenSize = m_editor->getScreenSize();
	glViewport(0, 0, screenSize.x, screenSize.y);
	glBindFramebuffer(GL_FRAMEBUFFER, previousFBO);

	// Copy viewport layers into prefab textures
	count = 0;
	for each (const auto & prefab in m_prefabs)
		glCopyImageSubData(m_viewport->m_gfxFBOS->getTexID("FXAA", 0), GL_TEXTURE_2D_ARRAY, 0, 0, 0, count++, prefab.texID, GL_TEXTURE_2D, 0, 0, 0, 0, m_thumbSize, m_thumbSize, 1);
}

void Prefabs::tickWindow(const float&)
{
	enum PrefabOptions {
		none,
		open,
		del,
		rename
	} prefabOption = none;

	// Draw Prefabs window
	if (ImGui::Begin("Prefabs", &m_open, ImGuiWindowFlags_AlwaysAutoResize)) {
		// Draw a search box with a refresh button
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

		// Display list of prefabs
		const auto directory = "\\Prefabs\\" + m_prefabSubDirectory;
		ImGui::Text(directory.c_str());
		ImGui::Spacing();
		auto columnCount = int(float(ImGui::GetWindowContentRegionMax().x) / float((ImGui::GetStyle().ItemSpacing.x * 2) + 50));
		columnCount = columnCount < 1 ? 1 : columnCount;
		ImGui::Columns(columnCount, nullptr, false);
		int count(0);
		for each (const auto & prefab in m_prefabs) {
			if (filter.PassFilter(prefab.name.c_str())) {
				ImGui::PushID(&prefab);
				GLuint textureID = prefab.texID;
				if (prefab.type == Entry::back && m_texBack->existsYet())
					textureID = m_texBack->m_glTexID;
				else if (prefab.type == Entry::folder && m_texFolder->existsYet())
					textureID = m_texFolder->m_glTexID;
				/*else if ((prefab.type == Entry::file) && m_texMissingThumb->existsYet())
					textureID = m_texMissingThumb->m_glTexID;*/

				ImGui::BeginGroup();
				ImVec4 color = count == m_selectedIndex ? ImVec4(1, 1, 1, 0.75) : ImVec4(0, 0, 0, 0);
				ImGui::ImageButton(
					(ImTextureID)static_cast<uintptr_t>(textureID),
					ImVec2(50, 50),
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
				else if (ImGui::IsItemHovered()) {
					m_hoverIndex = count;
					ImGui::BeginTooltip();
					ImGui::ImageButton(
						(ImTextureID)static_cast<uintptr_t>(textureID),
						ImVec2((float)m_thumbSize, (float)m_thumbSize),
						{ 0.0f, 1.0f }, { 1.0f, 0.0f },
						0,
						color
					);
					ImGui::EndTooltip();
				}
				ImGui::PopID();
				ImGui::NextColumn();
			}
			count++;
		}
	}
	ImGui::End();

	// Do something with the option chosen
	switch (prefabOption) {
	case open:
		openPrefabEntry();
		break;
	case del:
		ImGui::OpenPopup("Delete Prefab");
		break;
	case rename:
		ImGui::OpenPopup("Rename Prefab");
		break;
	}
}

void Prefabs::tickPopupDialogues(const float&)
{
	// Draw 'Delete Prefab' confirmation
	ImGui::SetNextWindowSize({ 350, 90 }, ImGuiCond_Appearing);
	if (ImGui::BeginPopupModal("Delete Prefab", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove)) {
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
			if (m_prefabs[m_selectedIndex].path.size()) {
				std::filesystem::remove(fullPath);
				populatePrefabs(m_prefabSubDirectory);
			}
			m_selectedIndex = -1;
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

	// Draw 'Rename Prefab' dialogue
	if (ImGui::BeginPopupModal("Rename Prefab", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove)) {
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