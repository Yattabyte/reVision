#include "Modules/Editor/Systems/Inspector_Prop_System.h"


Inspector_Prop_System::Inspector_Prop_System(Engine& engine, LevelEditor_Module& editor) noexcept :
	m_engine(engine),
	m_editor(editor)
{
	// Declare component types used
	addComponentType(Selected_Component::Runtime_ID);
	addComponentType(Prop_Component::Runtime_ID);

	populateModels();
}

void Inspector_Prop_System::updateComponents(const float& deltaTime, const std::vector<std::vector<ecsBaseComponent*>>& components) noexcept 
{
	ImGui::PushID(this);
	const auto text = std::string(Prop_Component::Name) + ": (" + std::to_string(components.size()) + ")";
	if (ImGui::CollapsingHeader(text.c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
		// Create list of handles for commands to use
		const auto getUUIDS = [&]() {
			std::vector<ComponentHandle> uuids;
			uuids.reserve(components.size());
			for (const auto& componentParam : components)
				uuids.push_back(componentParam[1]->m_handle);
			return uuids;
		};
		auto* propComponent = static_cast<Prop_Component*>(components[0][1]);

		static ImGuiTextFilter filter;
		ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 12.0f);
		filter.Draw("Filter");
		ImGui::PopStyleVar();

		auto typeInput = 0ull;
		std::vector<const char*> entries;
		entries.reserve(m_entries.size());
		auto x = 0ull;
		for (const auto& entry : m_entries) {
			if (filter.PassFilter(entry.c_str())) {
				entries.push_back(entry.c_str());
				if (propComponent->m_modelName == entry)
					typeInput = x;
			}
			++x;
		}
		if (entries.empty())
			entries.resize(1ull);
		static int item_current = static_cast<int>(typeInput);
		if (ImGui::Combo("Model File", &item_current, &entries[0], (int)entries.size())) {
			struct Name_Command final : Editor_Command {
				ecsWorld& m_ecsWorld;
				const std::vector<ComponentHandle> m_uuids;
				std::vector<std::string> m_oldData, m_newData;
				Name_Command(ecsWorld& world, const std::vector<ComponentHandle>& uuids, const std::string& data) noexcept
					: m_ecsWorld(world), m_uuids(uuids) {
					for (const auto& componentHandle : m_uuids) {
						if (const auto* component = m_ecsWorld.getComponent<Prop_Component>(componentHandle)) {
							m_oldData.push_back(component->m_modelName);
							m_newData.push_back(data);
						}
					}
				}
				void setData(const std::vector<std::string>& data) noexcept {
					if (data.size()) {
						size_t index(0ull);
						for (const auto& componentHandle : m_uuids) {
							if (auto* component = m_ecsWorld.getComponent<Prop_Component>(componentHandle)) {
								component->m_modelName = data[index++];
								component->m_model.reset();
								component->m_uploadModel = false;
								component->m_uploadMaterial = false;
								component->m_offset = 0ull;
								component->m_count = 0ull;
								component->m_materialID = 0u;
							}
						}
					}
				}
				virtual void execute() noexcept override final {
					setData(m_newData);
				}
				virtual void undo() noexcept override final {
					setData(m_oldData);
				}
				virtual bool join(Editor_Command* other) noexcept override final {
					if (auto newCommand = dynamic_cast<Name_Command*>(other)) {
						if (std::equal(m_uuids.cbegin(), m_uuids.cend(), newCommand->m_uuids.cbegin())) {
							m_newData = newCommand->m_newData;
							return true;
						}
					}
					return false;
				}
			};
			item_current = std::clamp(item_current, 0, (int)m_entries.size());
			m_editor.doReversableAction(std::make_shared<Name_Command>(m_editor.getWorld(), getUUIDS(), m_entries[item_current]));
		}

		auto skinInput = (int)(propComponent->m_skin);
		if (ImGui::DragInt("Skin", &skinInput)) {
			struct Skin_Command final : Editor_Command {
				ecsWorld& m_ecsWorld;
				const std::vector<ComponentHandle> m_uuids;
				std::vector<unsigned int> m_oldData, m_newData;
				Skin_Command(ecsWorld& world, const std::vector<ComponentHandle>& uuids, const unsigned int& data) noexcept
					: m_ecsWorld(world), m_uuids(uuids) {
					for (const auto& componentHandle : m_uuids) {
						if (const auto* component = m_ecsWorld.getComponent<Prop_Component>(componentHandle)) {
							m_oldData.push_back(component->m_skin);
							m_newData.push_back(data);
						}
					}
				}
				void setData(const std::vector<unsigned int>& data) noexcept {
					if (data.size()) {
						size_t index(0ull);
						for (const auto& componentHandle : m_uuids) {
							if (auto* component = m_ecsWorld.getComponent<Prop_Component>(componentHandle))
								component->m_skin = data[index++];
						}
					}
				}
				virtual void execute() noexcept override final {
					setData(m_newData);
				}
				virtual void undo() noexcept override final {
					setData(m_oldData);
				}
				virtual bool join(Editor_Command* other) noexcept override final {
					if (auto newCommand = dynamic_cast<Skin_Command*>(other)) {
						if (std::equal(m_uuids.cbegin(), m_uuids.cend(), newCommand->m_uuids.cbegin())) {
							m_newData = newCommand->m_newData;
							return true;
						}
					}
					return false;
				}
			};
			m_editor.doReversableAction(std::make_shared<Skin_Command>(m_editor.getWorld(), getUUIDS(), skinInput));
		}
		for (const auto& componentParam : components)
			static_cast<Prop_Component*>(componentParam[1])->m_skin = (unsigned int)skinInput;
	}
	ImGui::PopID();
}

void Inspector_Prop_System::populateModels() 
{
	// Delete the entries
	m_entries.clear();

	const auto rootPath = Engine::Get_Current_Dir() + "\\Models\\";
	const auto path = std::filesystem::path(rootPath);
	const auto types = Mesh_IO::Get_Supported_Types();

	// Cycle through each entry on disk, making prefab entries
	for (auto& entry : std::filesystem::recursive_directory_iterator(path))
		if (entry.is_regular_file() && entry.path().has_extension() && std::any_of(types.cbegin(), types.cend(), [&](const auto& s) {return entry.path().extension().string() == s; }))
			m_entries.push_back(std::filesystem::relative(entry, rootPath).string());
}