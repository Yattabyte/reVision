#pragma once
#ifndef INSPECTOR_COLLIDER_SYSTEM_H
#define INSPECTOR_COLLIDER_SYSTEM_H

#include "Modules/Editor/Editor_M.h"
#include "Modules/ECS/ecsSystem.h"
#include "Modules/ECS/component_types.h"
#include "Utilities/IO/Mesh_IO.h"
#include "imgui.h"
#include "glm/gtc/type_ptr.hpp"
#include "Engine.h"
#include <algorithm>
#include <filesystem>


/***/
class Inspector_Collider_System final : public ecsBaseSystem {
public:
	// Public (De)Constructors
	/** Destroy this system. */
	inline ~Inspector_Collider_System() = default;
	/** Construct this system. */
	inline Inspector_Collider_System(Engine* engine, LevelEditor_Module* editor) noexcept :
		m_engine(engine),
		m_editor(editor)
	{
		// Declare component types used
		addComponentType(Selected_Component::Runtime_ID);
		addComponentType(Collider_Component::Runtime_ID);

		populateModels();
	}


	// Public Interface Implementation
	inline virtual void updateComponents(const float& deltaTime, const std::vector<std::vector<ecsBaseComponent*>>& components) noexcept override final {
		ImGui::PushID(this);
		const auto text = std::string(Collider_Component::Name) + ": (" + std::to_string(components.size()) + ")";
		if (ImGui::CollapsingHeader(text.c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
			// Create list of handles for commands to use
			const auto getUUIDS = [&]() {
				std::vector<ComponentHandle> uuids;
				uuids.reserve(components.size());
				for (const auto& componentParam : components)
					uuids.push_back(componentParam[1]->m_handle);
				return uuids;
			};
			auto* colliderComponent = static_cast<Collider_Component*>(components[0][1]);

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
					if (colliderComponent->m_modelName == entry)
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
				m_editor->doReversableAction(std::make_shared<Name_Command>(m_editor->getWorld(), getUUIDS(), m_entries[item_current]));
			}

			auto restitutionInput = colliderComponent->m_restitution;
			if (ImGui::DragFloat("Restitution", &restitutionInput, 0.1f, 0.0f, 1.0f)) {
				struct Restitution_Command final : Editor_Command {
					ecsWorld& m_ecsWorld;
					const std::vector<ComponentHandle> m_uuids;
					std::vector<float> m_oldData, m_newData;
					Restitution_Command(ecsWorld& world, const std::vector<ComponentHandle>& uuids, const float& data) noexcept
						: m_ecsWorld(world), m_uuids(uuids) {
						for (const auto& componentHandle : m_uuids) {
							if (const auto* component = m_ecsWorld.getComponent<Collider_Component>(componentHandle)) {
								m_oldData.push_back(component->m_restitution);
								m_newData.push_back(data);
							}
						}
					}
					void setData(const std::vector<float>& data) noexcept {
						if (data.size()) {
							size_t index(0ull);
							for (const auto& componentHandle : m_uuids) {
								if (auto* component = m_ecsWorld.getComponent<Collider_Component>(componentHandle))
									component->m_restitution = data[index++];
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
						if (auto newCommand = dynamic_cast<Restitution_Command*>(other)) {
							if (std::equal(m_uuids.cbegin(), m_uuids.cend(), newCommand->m_uuids.cbegin())) {
								m_newData = newCommand->m_newData;
								return true;
							}
						}
						return false;
					}
				};
				m_editor->doReversableAction(std::make_shared<Restitution_Command>(m_editor->getWorld(), getUUIDS(), restitutionInput));
			}

			auto frictionInput = colliderComponent->m_friction;
			if (ImGui::DragFloat("Friction", &frictionInput, 0.1f, 0.0f, 1.0f)) {
				struct Friction_Command final : Editor_Command {
					ecsWorld& m_ecsWorld;
					const std::vector<ComponentHandle> m_uuids;
					std::vector<float> m_oldData, m_newData;
					Friction_Command(ecsWorld& world, const std::vector<ComponentHandle>& uuids, const float& data) noexcept
						: m_ecsWorld(world), m_uuids(uuids) {
						for (const auto& componentHandle : m_uuids) {
							if (const auto* component = m_ecsWorld.getComponent<Collider_Component>(componentHandle)) {
								m_oldData.push_back(component->m_friction);
								m_newData.push_back(data);
							}
						}
					}
					void setData(const std::vector<float>& data) noexcept {
						if (data.size()) {
							size_t index(0ull);
							for (const auto& componentHandle : m_uuids) {
								if (auto* component = m_ecsWorld.getComponent<Collider_Component>(componentHandle))
									component->m_friction = data[index++];
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
						if (auto newCommand = dynamic_cast<Friction_Command*>(other)) {
							if (std::equal(m_uuids.cbegin(), m_uuids.cend(), newCommand->m_uuids.cbegin())) {
								m_newData = newCommand->m_newData;
								return true;
							}
						}
						return false;
					}
				};
				m_editor->doReversableAction(std::make_shared<Friction_Command>(m_editor->getWorld(), getUUIDS(), frictionInput));
			}

			auto massInput = colliderComponent->m_mass;
			if (ImGui::DragFloat("Mass", &massInput)) {
				struct Mass_Command final : Editor_Command {
					ecsWorld& m_ecsWorld;
					const std::vector<ComponentHandle> m_uuids;
					std::vector<float> m_oldData, m_newData;
					Mass_Command(ecsWorld& world, const std::vector<ComponentHandle>& uuids, const float& data) noexcept
						: m_ecsWorld(world), m_uuids(uuids) {
						for (const auto& componentHandle : m_uuids) {
							if (const auto* component = m_ecsWorld.getComponent<Collider_Component>(componentHandle)) {
								m_oldData.push_back(component->m_mass);
								m_newData.push_back(data);
							}
						}
					}
					void setData(const std::vector<float>& data) noexcept {
						if (data.size()) {
							size_t index(0ull);
							for (const auto& componentHandle : m_uuids) {
								if (auto* component = m_ecsWorld.getComponent<Collider_Component>(componentHandle))
									component->m_mass = data[index++];
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
						if (auto newCommand = dynamic_cast<Mass_Command*>(other)) {
							if (std::equal(m_uuids.cbegin(), m_uuids.cend(), newCommand->m_uuids.cbegin())) {
								m_newData = newCommand->m_newData;
								return true;
							}
						}
						return false;
					}
				};
				m_editor->doReversableAction(std::make_shared<Mass_Command>(m_editor->getWorld(), getUUIDS(), massInput));
			}
		}
		ImGui::PopID();
	}


private:
	// Private Methods
	/** Populates an internal list of models found & supported recursively within the application's models directory. */
	void populateModels() {
		// Delete the entries
		m_entries.clear();

		const auto rootPath = Engine::Get_Current_Dir() + "\\Models\\";
		const auto path = std::filesystem::path(rootPath);
		const auto types = Mesh_IO::Get_Supported_Types();
		
		// Cycle through each entry on disk, making prefab entries
		for (auto& entry : std::filesystem::recursive_directory_iterator(path)) 
			if (entry.is_regular_file() && entry.path().has_extension() && std::any_of(types.cbegin(), types.cend(), [&](const auto& s) {return entry.path().extension().string() == s;}))
				m_entries.push_back(std::filesystem::relative(entry, rootPath).string());
	}


	// Private Attributes
	Engine* m_engine = nullptr;
	LevelEditor_Module* m_editor = nullptr;
	std::vector<std::string> m_entries;
};

#endif // INSPECTOR_COLLIDER_SYSTEM_H
