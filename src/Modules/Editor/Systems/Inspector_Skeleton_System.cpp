#include "Modules/Editor/Systems/Inspector_Skeleton_System.h"
#include "Modules/Editor/Editor_M.h"
#include "Modules/ECS/component_types.h"
#include "Utilities/IO/Mesh_IO.h"
#include "Engine.h"
#include "imgui.h"
#include <filesystem>


Inspector_Skeleton_System::Inspector_Skeleton_System(Engine& engine, LevelEditor_Module& editor) noexcept :
	m_engine(engine),
	m_editor(editor)
{
	// Declare component types used
	addComponentType(Selected_Component::Runtime_ID);
	addComponentType(Skeleton_Component::Runtime_ID);

	populateModels();
}

void Inspector_Skeleton_System::updateComponents(const float&, const std::vector<std::vector<ecsBaseComponent*>>& components) noexcept
{
	ImGui::PushID(this);
	const auto text = std::string(Skeleton_Component::Name) + ": (" + std::to_string(components.size()) + ")";
	if (ImGui::CollapsingHeader(text.c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
		// Create list of handles for commands to use
		const auto getUUIDS = [&]() {
			std::vector<ComponentHandle> uuids;
			uuids.reserve(components.size());
			for (const auto& componentParam : components)
				uuids.push_back(componentParam[1]->m_handle);
			return uuids;
		};
		const auto* skeletonComponent = static_cast<Skeleton_Component*>(components[0][1]);

		auto animationInput = skeletonComponent->m_animation;
		if (ImGui::DragInt("Animation ID", &animationInput)) {
			struct Animation_Command final : Editor_Command {
				ecsWorld& m_ecsWorld;
				const std::vector<ComponentHandle> m_uuids;
				std::vector<int> m_oldData, m_newData;
				Animation_Command(ecsWorld& world, const std::vector<ComponentHandle>& uuids, const int& data) noexcept
					: m_ecsWorld(world), m_uuids(uuids) {
					for (const auto& componentHandle : m_uuids) {
						if (const auto* component = m_ecsWorld.getComponent<Skeleton_Component>(componentHandle)) {
							m_oldData.push_back(component->m_animation);
							m_newData.push_back(data);
						}
					}
				}
				void setData(const std::vector<int>& data) noexcept {
					if (data.size()) {
						size_t index(0ull);
						for (const auto& componentHandle : m_uuids) {
							if (auto* component = m_ecsWorld.getComponent<Skeleton_Component>(componentHandle))
								component->m_animation = data[index++];
						}
					}
				}
				void execute() noexcept final {
					setData(m_newData);
				}
				void undo() noexcept final {
					setData(m_oldData);
				}
				bool join(Editor_Command* other) noexcept final {
					if (const auto& newCommand = dynamic_cast<Animation_Command*>(other)) {
						if (std::equal(m_uuids.cbegin(), m_uuids.cend(), newCommand->m_uuids.cbegin())) {
							m_newData = newCommand->m_newData;
							return true;
						}
					}
					return false;
				}
			};
			m_editor.doReversableAction(std::make_shared<Animation_Command>(m_editor.getWorld(), getUUIDS(), animationInput));
		}

		auto playInput = skeletonComponent->m_playAnim;
		if (ImGui::Checkbox("Play Animation", &playInput)) {
			struct PlayAnimation_Command final : Editor_Command {
				ecsWorld& m_ecsWorld;
				const std::vector<ComponentHandle> m_uuids;
				std::vector<bool> m_oldData, m_newData;
				PlayAnimation_Command(ecsWorld& world, const std::vector<ComponentHandle>& uuids, const bool& data) noexcept
					: m_ecsWorld(world), m_uuids(uuids) {
					for (const auto& componentHandle : m_uuids) {
						if (const auto* component = m_ecsWorld.getComponent<Skeleton_Component>(componentHandle)) {
							m_oldData.push_back(component->m_playAnim);
							m_newData.push_back(data);
						}
					}
				}
				void setData(const std::vector<bool>& data) noexcept {
					if (data.size()) {
						size_t index(0ull);
						for (const auto& componentHandle : m_uuids) {
							if (auto* component = m_ecsWorld.getComponent<Skeleton_Component>(componentHandle))
								component->m_playAnim = data[index++];
						}
					}
				}
				void execute() noexcept final {
					setData(m_newData);
				}
				void undo() noexcept final {
					setData(m_oldData);
				}
				bool join(Editor_Command* other) noexcept final {
					if (const auto& newCommand = dynamic_cast<PlayAnimation_Command*>(other)) {
						if (std::equal(m_uuids.cbegin(), m_uuids.cend(), newCommand->m_uuids.cbegin())) {
							m_newData = newCommand->m_newData;
							return true;
						}
					}
					return false;
				}
			};
			m_editor.doReversableAction(std::make_shared<PlayAnimation_Command>(m_editor.getWorld(), getUUIDS(), playInput));
		}
	}
	ImGui::PopID();
}

void Inspector_Skeleton_System::populateModels()
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