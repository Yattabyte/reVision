#pragma once
#ifndef INSPECTOR_PROP_SYSTEM_H
#define INSPECTOR_PROP_SYSTEM_H 

#include "Modules/Editor/Editor_M.h"
#include "Modules/World/World_M.h"
#include "Modules/ECS/ecsSystem.h"
#include "Modules/ECS/component_types.h"
#include "Modules/UI/dear imgui/imgui.h"
#include "glm/gtc/type_ptr.hpp"
#include "Engine.h"


/** An ECS system allowing the user to inpsect selected prop components. */
class Inspector_Prop_System final : public ecsBaseSystem {
public:
	// Public (de)Constructors
	/** Destroy this system. */
	inline ~Inspector_Prop_System() = default;
	/** Construct this system. */
	inline Inspector_Prop_System(Engine* engine, LevelEditor_Module* editor)
		: m_engine(engine), m_editor(editor) {
		// Declare component types used
		addComponentType(Selected_Component::m_ID);
		addComponentType(Prop_Component::m_ID);
	}


	// Public Interface Implementation
	inline virtual void updateComponents(const float& deltaTime, const std::vector<std::vector<ecsBaseComponent*>>& components) override final {
		const auto text = std::string(Prop_Component::m_name) + ": (" + std::to_string(components.size()) + ")";
		if (ImGui::CollapsingHeader(text.c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
			// Create list of handles for commands to use
			const auto getUUIDS = [&]() {
				std::vector<ecsHandle> uuids;
				uuids.reserve(components.size());
				for each (const auto & componentParam in components)
					uuids.push_back(componentParam[0]->m_entity);
				return uuids;
			};

			char nameInput[256];
			for (size_t x = 0; x < ((Prop_Component*)components[0][1])->m_modelName.length() && x < IM_ARRAYSIZE(nameInput); ++x)
				nameInput[x] = ((Prop_Component*)components[0][1])->m_modelName[x];
			nameInput[std::min(256ull, ((Prop_Component*)components[0][1])->m_modelName.length())] = '\0';
			if (ImGui::InputText("Model Name", nameInput, IM_ARRAYSIZE(nameInput))) {
				struct Name_Command final : Editor_Command {
					ecsWorld& m_ecsWorld;
					const std::vector<ecsHandle> m_uuids;
					std::vector<std::string> m_oldData, m_newData;
					Name_Command(ecsWorld& world, const std::vector<ecsHandle>& uuids, const std::string& data)
						: m_ecsWorld(world), m_uuids(uuids) {
						for each (const auto & entityHandle in m_uuids) {
							const auto* component = m_ecsWorld.getComponent<Prop_Component>(entityHandle);
							m_oldData.push_back(component->m_modelName);
							m_newData.push_back(data);
						}
					}
					void setData(const std::vector<std::string>& data) {
						if (data.size()) {
							size_t index(0ull);
							for each (const auto & entityHandle in m_uuids) {
								auto* component = m_ecsWorld.getComponent<Prop_Component>(entityHandle);
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
					virtual void execute() override final {
						setData(m_newData);
					}
					virtual void undo() override final {
						setData(m_oldData);
					}
					virtual bool join(Editor_Command* const other) override final {
						if (auto newCommand = dynamic_cast<Name_Command*>(other)) {
							if (std::equal(m_uuids.cbegin(), m_uuids.cend(), newCommand->m_uuids.cbegin())) {
								m_newData = newCommand->m_newData;
								return true;
							}
						}
						return false;
					}
				};
				m_editor->doReversableAction(std::make_shared<Name_Command>(m_engine->getModule_ECS().getWorld(), getUUIDS(), std::string(nameInput)));
			}

			auto skinInput = (int)((Prop_Component*)components[0][1])->m_skin;
			if (ImGui::DragInt("Skin", &skinInput)) {
				struct Skin_Command final : Editor_Command {
					ecsWorld& m_ecsWorld;
					const std::vector<ecsHandle> m_uuids;
					std::vector<unsigned int> m_oldData, m_newData;
					Skin_Command(ecsWorld& world, const std::vector<ecsHandle>& uuids, const unsigned int& data)
						: m_ecsWorld(world), m_uuids(uuids) {
						for each (const auto & entityHandle in m_uuids) {
							const auto* component = m_ecsWorld.getComponent<Prop_Component>(entityHandle);
							m_oldData.push_back(component->m_skin);
							m_newData.push_back(data);
						}
					}
					void setData(const std::vector<unsigned int>& data) {
						if (data.size()) {
							size_t index(0ull);
							for each (const auto & entityHandle in m_uuids) {
								auto* component = m_ecsWorld.getComponent<Prop_Component>(entityHandle);
								component->m_skin = data[index++];
							}
						}
					}
					virtual void execute() override final {
						setData(m_newData);
					}
					virtual void undo() override final {
						setData(m_oldData);
					}
					virtual bool join(Editor_Command* const other) override final {
						if (auto newCommand = dynamic_cast<Skin_Command*>(other)) {
							if (std::equal(m_uuids.cbegin(), m_uuids.cend(), newCommand->m_uuids.cbegin())) {
								m_newData = newCommand->m_newData;
								return true;
							}
						}
						return false;
					}
				};
				m_editor->doReversableAction(std::make_shared<Skin_Command>(m_engine->getModule_ECS().getWorld(), getUUIDS(), skinInput));
			}
				for each (auto & componentParam in components)
					((Prop_Component*)componentParam[1])->m_skin = (unsigned int)skinInput;
		}
	}


private:
	// Private Attributes
	Engine* m_engine = nullptr;
	LevelEditor_Module* m_editor = nullptr;
};
#endif // INSPECTOR_PROP_SYSTEM_H