#pragma once
#ifndef INSPECTOR_LIGHTCOLOR_SYSTEM_H
#define INSPECTOR_LIGHTCOLOR_SYSTEM_H

#include "Modules/Editor/Editor_M.h"
#include "Modules/ECS/ecsSystem.h"
#include "Modules/ECS/component_types.h"
#include "imgui.h"
#include "glm/gtc/type_ptr.hpp"
#include "Engine.h"


/** An ECS system allowing the user to inspect selected light color components. */
class Inspector_LightColor_System final : public ecsBaseSystem {
public:
	// Public (de)Constructors
	/** Destroy this system. */
	inline ~Inspector_LightColor_System() = default;
	/** Construct this system. */
	inline Inspector_LightColor_System(Engine* engine, LevelEditor_Module* editor)
		: m_engine(engine), m_editor(editor) {
		// Declare component types used
		addComponentType(Selected_Component::m_ID);
		addComponentType(LightColor_Component::m_ID);
	}


	// Public Interface Implementation
	inline virtual void updateComponents(const float& deltaTime, const std::vector<std::vector<ecsBaseComponent*>>& components) override final {
		const auto text = std::string(LightColor_Component::m_name) + ": (" + std::to_string(components.size()) + ")";
		if (ImGui::CollapsingHeader(text.c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
			// Create list of handles for commands to use
			const auto getUUIDS = [&]() {
				std::vector<ComponentHandle> uuids;
				uuids.reserve(components.size());
				for each (const auto & componentParam in components)
					uuids.push_back(componentParam[1]->m_handle);
				return uuids;
			};

			auto colorInput = ((LightColor_Component*)components[0][1])->m_color;
			if (ImGui::ColorEdit3("Color", glm::value_ptr(colorInput))) {
				struct Color_Command final : Editor_Command {
					ecsWorld& m_ecsWorld;
					const std::vector<ComponentHandle> m_uuids;
					std::vector<glm::vec3> m_oldData, m_newData;
					Color_Command(ecsWorld& world, const std::vector<ComponentHandle>& uuids, const glm::vec3& data)
						: m_ecsWorld(world), m_uuids(uuids) {
						for each (const auto & componentHandle in m_uuids) {
							if (const auto* component = m_ecsWorld.getComponent<LightColor_Component>(componentHandle)) {
								m_oldData.push_back(component->m_color);
								m_newData.push_back(data);
							}
						}
					}
					void setData(const std::vector<glm::vec3>& data) {
						if (data.size()) {
							size_t index(0ull);
							for each (const auto & componentHandle in m_uuids)
								if (auto* component = m_ecsWorld.getComponent<LightColor_Component>(componentHandle))
									component->m_color = data[index++];
						}
					}
					virtual void execute() override final {
						setData(m_newData);
					}
					virtual void undo() override final {
						setData(m_oldData);
					}
					virtual bool join(Editor_Command* const other) override final {
						if (auto newCommand = dynamic_cast<Color_Command*>(other)) {
							if (std::equal(m_uuids.cbegin(), m_uuids.cend(), newCommand->m_uuids.cbegin())) {
								m_newData = newCommand->m_newData;
								return true;
							}
						}
						return false;
					}
				};
				m_editor->doReversableAction(std::make_shared<Color_Command>(m_editor->getActiveWorld(), getUUIDS(), colorInput));
			}

			auto intensityInput = ((LightColor_Component*)(components[0][1]))->m_intensity;
			if (ImGui::DragFloat("Intensity", &intensityInput)) {
				struct Intensity_Command final : Editor_Command {
					ecsWorld& m_ecsWorld;
					const std::vector<ComponentHandle> m_uuids;
					std::vector<float> m_oldData, m_newData;
					Intensity_Command(ecsWorld& world, const std::vector<ComponentHandle>& uuids, const float& data)
						: m_ecsWorld(world), m_uuids(uuids) {
						for each (const auto & componentHandle in m_uuids) {
							if (const auto* component = m_ecsWorld.getComponent<LightColor_Component>(componentHandle)) {
								m_oldData.push_back(component->m_intensity);
								m_newData.push_back(data);
							}
						}
					}
					void setData(const std::vector<float>& data) {
						if (data.size()) {
							size_t index(0ull);
							for each (const auto & componentHandle in m_uuids) {
								if (auto* component = m_ecsWorld.getComponent<LightColor_Component>(componentHandle))
									component->m_intensity = data[index++];
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
						if (auto newCommand = dynamic_cast<Intensity_Command*>(other)) {
							if (std::equal(m_uuids.cbegin(), m_uuids.cend(), newCommand->m_uuids.cbegin())) {
								m_newData = newCommand->m_newData;
								return true;
							}
						}
						return false;
					}
				};
				m_editor->doReversableAction(std::make_shared<Intensity_Command>(m_editor->getActiveWorld(), getUUIDS(), intensityInput));
			}
		}
	}


private:
	// Private Attributes
	Engine* m_engine = nullptr;
	LevelEditor_Module* m_editor = nullptr;
};

#endif // INSPECTOR_LIGHTCOLOR_SYSTEM_H