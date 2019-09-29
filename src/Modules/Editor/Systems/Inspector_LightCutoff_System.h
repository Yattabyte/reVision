#pragma once
#ifndef INSPECTOR_LIGHTCUTOFF_SYSTEM_H
#define INSPECTOR_LIGHTCUTOFF_SYSTEM_H

#include "Modules/Editor/Editor_M.h"
#include "Modules/World/World_M.h"
#include "Modules/ECS/ecsSystem.h"
#include "Modules/ECS/component_types.h"
#include "imgui.h"
#include "glm/gtc/type_ptr.hpp"
#include "Engine.h"


/** An ECS system allowing the user to inspect selected light cutoff components. */
class Inspector_LightCutoff_System final : public ecsBaseSystem {
public:
	// Public (de)Constructors
	/** Destroy this system. */
	inline ~Inspector_LightCutoff_System() = default;
	/** Construct this system. */
	inline Inspector_LightCutoff_System(Engine* engine, LevelEditor_Module* editor)
		: m_engine(engine), m_editor(editor) {
		// Declare component types used
		addComponentType(Selected_Component::m_ID);
		addComponentType(LightCutoff_Component::m_ID);
	}


	// Public Interface Implementation
	inline virtual void updateComponents(const float& deltaTime, const std::vector<std::vector<ecsBaseComponent*>>& components) override final {
		const auto text = std::string(LightCutoff_Component::m_name) + ": (" + std::to_string(components.size()) + ")";
		if (ImGui::CollapsingHeader(text.c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
			// Create list of handles for commands to use
			const auto getUUIDS = [&]() {
				std::vector<ecsHandle> uuids;
				uuids.reserve(components.size());
				for each (const auto & componentParam in components)
					uuids.push_back(componentParam[0]->m_entity);
				return uuids;
			};

			auto cutoffInput = ((LightCutoff_Component*)components[0][1])->m_cutoff;
			if (ImGui::DragFloat("Cutoff", &cutoffInput)) {
				struct Cutoff_Command final : Editor_Command {
					ecsWorld& m_ecsWorld;
					const std::vector<ecsHandle> m_uuids;
					std::vector<float> m_oldData, m_newData;
					Cutoff_Command(ecsWorld& world, const std::vector<ecsHandle>& uuids, const float& data)
						: m_ecsWorld(world), m_uuids(uuids) {
						for each (const auto & entityHandle in m_uuids) {
							const auto* component = m_ecsWorld.getComponent<LightCutoff_Component>(entityHandle);
							m_oldData.push_back(component->m_cutoff);
							m_newData.push_back(data);
						}
					}
					void setData(const std::vector<float>& data) {
						if (data.size()) {
							size_t index(0ull);
							for each (const auto & entityHandle in m_uuids) {
								auto* component = m_ecsWorld.getComponent<LightCutoff_Component>(entityHandle);
								component->m_cutoff = data[index++];
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
						if (auto newCommand = dynamic_cast<Cutoff_Command*>(other)) {
							if (std::equal(m_uuids.cbegin(), m_uuids.cend(), newCommand->m_uuids.cbegin())) {
								m_newData = newCommand->m_newData;
								return true;
							}
						}
						return false;
					}
				};
				m_editor->doReversableAction(std::make_shared<Cutoff_Command>(m_engine->getModule_ECS().getWorld(), getUUIDS(), cutoffInput));
			}
		}
	}


private:
	// Private Attributes
	Engine* m_engine = nullptr;
	LevelEditor_Module* m_editor = nullptr;
};
#endif // INSPECTOR_LIGHTCUTOFF_SYSTEM_H