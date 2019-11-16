#pragma once
#ifndef INSPECTOR_PROP_SYSTEM_H
#define INSPECTOR_PROP_SYSTEM_H

#include "Modules/Editor/Editor_M.h"
#include "Modules/ECS/ecsSystem.h"
#include "Modules/ECS/component_types.h"
#include "imgui.h"
#include "glm/gtc/type_ptr.hpp"
#include "Engine.h"


/** An ECS system allowing the user to inspect selected prop components. */
class Inspector_Prop_System final : public ecsBaseSystem {
public:
	// Public (De)Constructors
	/** Destroy this system. */
	inline ~Inspector_Prop_System() = default;
	/** Construct this system. */
	inline Inspector_Prop_System(Engine* engine, LevelEditor_Module* editor) noexcept :
		m_engine(engine),
		m_editor(editor)
	{
		// Declare component types used
		addComponentType(Selected_Component::Runtime_ID);
		addComponentType(Prop_Component::Runtime_ID);
	}


	// Public Interface Implementation
	inline virtual void updateComponents(const float& deltaTime, const std::vector<std::vector<ecsBaseComponent*>>& components) noexcept override final {
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

			char nameInput[256];
			for (size_t x = 0; x < (propComponent)->m_modelName.length() && x < IM_ARRAYSIZE(nameInput); ++x)
				nameInput[x] = (propComponent)->m_modelName[x];
			nameInput[std::min(256ull, propComponent->m_modelName.length())] = '\0';
			if (ImGui::InputText("Model Name", nameInput, IM_ARRAYSIZE(nameInput))) {
				struct Name_Command final : Editor_Command {
					ecsWorld& m_ecsWorld;
					const std::vector<ComponentHandle> m_uuids;
					std::vector<std::string> m_oldData, m_newData;
					Name_Command(ecsWorld& world, const std::vector<ComponentHandle>& uuids, const std::string& data) noexcept
						: m_ecsWorld(world), m_uuids(uuids) {
						for (const auto & componentHandle : m_uuids) {
							if (const auto* component = m_ecsWorld.getComponent<Prop_Component>(componentHandle)) {
								m_oldData.push_back(component->m_modelName);
								m_newData.push_back(data);
							}
						}
					}
					void setData(const std::vector<std::string>& data) noexcept {
						if (data.size()) {
							size_t index(0ull);
							for (const auto & componentHandle : m_uuids) {
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
				m_editor->doReversableAction(std::make_shared<Name_Command>(m_editor->getWorld(), getUUIDS(), std::string(nameInput)));
			}

			auto skinInput = (int)(propComponent->m_skin);
			if (ImGui::DragInt("Skin", &skinInput)) {
				struct Skin_Command final : Editor_Command {
					ecsWorld& m_ecsWorld;
					const std::vector<ComponentHandle> m_uuids;
					std::vector<unsigned int> m_oldData, m_newData;
					Skin_Command(ecsWorld& world, const std::vector<ComponentHandle>& uuids, const unsigned int& data) noexcept
						: m_ecsWorld(world), m_uuids(uuids) {
						for (const auto & componentHandle : m_uuids) {
							if (const auto* component = m_ecsWorld.getComponent<Prop_Component>(componentHandle)) {
								m_oldData.push_back(component->m_skin);
								m_newData.push_back(data);
							}
						}
					}
					void setData(const std::vector<unsigned int>& data) noexcept {
						if (data.size()) {
							size_t index(0ull);
							for (const auto & componentHandle : m_uuids) {
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
				m_editor->doReversableAction(std::make_shared<Skin_Command>(m_editor->getWorld(), getUUIDS(), skinInput));
			}
			for (const auto& componentParam : components)
				static_cast<Prop_Component*>(componentParam[1])->m_skin = (unsigned int)skinInput;
		}
	}


private:
	// Private Attributes
	Engine* m_engine = nullptr;
	LevelEditor_Module* m_editor = nullptr;
};

#endif // INSPECTOR_PROP_SYSTEM_H