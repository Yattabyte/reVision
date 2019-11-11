#pragma once
#ifndef INSPECTOR_TRANSFORM_SYSTEM_H
#define INSPECTOR_TRANSFORM_SYSTEM_H

#include "Modules/Editor/Editor_M.h"
#include "Modules/ECS/ecsSystem.h"
#include "Modules/ECS/component_types.h"
#include "imgui.h"
#include "glm/gtc/type_ptr.hpp"
#include "Engine.h"


/** An ECS system allowing the user to inspect selected component transforms.*/
class Inspector_Transform_System final : public ecsBaseSystem {
public:
	// Public (de)Constructors
	/** Destroy this system. */
	inline ~Inspector_Transform_System() = default;
	/** Construct this system.
	@param	editor		the level editor. */
	inline Inspector_Transform_System(Engine* engine, LevelEditor_Module* editor)
		: m_engine(engine), m_editor(editor) {
		// Declare component types used
		addComponentType(Selected_Component::m_ID);
		addComponentType(Transform_Component::m_ID);
	}


	// Public Interface Implementation
	inline virtual void updateComponents(const float& deltaTime, const std::vector<std::vector<ecsBaseComponent*>>& components) override final {
		const auto text = std::string(Transform_Component::m_name) + ": (" + std::to_string(components.size()) + ")";
		if (ImGui::CollapsingHeader(text.c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
			// Create list of handles for commands to use
			const auto getUUIDS = [&]() {
				std::vector<ComponentHandle> uuids;
				uuids.reserve(components.size());
				for each (const auto & componentParam in components)
					uuids.push_back(componentParam[1]->m_handle);
				return uuids;
			};
			auto* transComponent = static_cast<Transform_Component*>(components[0][1]);

			// Position
			auto& posInput = transComponent->m_localTransform.m_position;
			if (ImGui::DragFloat3("Position", glm::value_ptr(posInput))) {
				struct Move_Command final : Editor_Command {
					ecsWorld& m_ecsWorld;
					LevelEditor_Module& m_editor;
					const std::vector<ComponentHandle> m_uuids;
					std::vector<glm::vec3> m_oldData, m_newData;
					Move_Command(ecsWorld& world, LevelEditor_Module& editor, const std::vector<ComponentHandle>& uuids, const glm::vec3& newPosition)
						: m_ecsWorld(world), m_editor(editor), m_uuids(uuids) {
						for each (const auto & componentHandle in m_uuids) {
							if (const auto* component = m_ecsWorld.getComponent<Transform_Component>(componentHandle)) {
								m_oldData.push_back(component->m_localTransform.m_position);
								m_newData.push_back(newPosition);
							}
						}
					}
					void setPosition(const std::vector<glm::vec3>& positions) {
						if (positions.size()) {
							size_t index(0ull);
							for each (const auto & componentHandle in m_uuids) {
								if (auto* component = m_ecsWorld.getComponent<Transform_Component>(componentHandle)) {
									component->m_localTransform.m_position = positions[index++];
									component->m_localTransform.update();
								}
							}
							auto newTransform = m_editor.getGizmoTransform();
							newTransform.m_position = positions[0];
							newTransform.update();
							m_editor.setGizmoTransform(newTransform);
						}
					}
					virtual void execute() override final {
						setPosition(m_newData);
					}
					virtual void undo() override final {
						setPosition(m_oldData);
					}
					virtual bool join(Editor_Command* const other) override final {
						if (auto newCommand = dynamic_cast<Move_Command*>(other)) {
							if (std::equal(m_uuids.cbegin(), m_uuids.cend(), newCommand->m_uuids.cbegin())) {
								m_newData = newCommand->m_newData;
								return true;
							}
						}
						return false;
					}
				};
				m_editor->doReversableAction(std::make_shared<Move_Command>(m_editor->getWorld(), *m_editor, getUUIDS(), posInput));
			}

			// Rotation
			auto rotInput = glm::degrees(glm::eulerAngles(transComponent->m_localTransform.m_orientation));
			if (ImGui::DragFloat3("Rotation", glm::value_ptr(rotInput))) {
				struct Rotate_Command final : Editor_Command {
					ecsWorld& m_ecsWorld;
					const std::vector<ComponentHandle> m_uuids;
					std::vector<glm::quat> m_oldData, m_newData;
					Rotate_Command(ecsWorld& world, const std::vector<ComponentHandle>& uuids, const glm::quat& newOrientation)
						: m_ecsWorld(world), m_uuids(uuids) {
						for each (const auto & componentHandle in m_uuids) {
							if (const auto* component = m_ecsWorld.getComponent<Transform_Component>(componentHandle)) {
								m_oldData.push_back(component->m_localTransform.m_orientation);
								m_newData.push_back(newOrientation);
							}
						}
					}
					void setOrientation(const std::vector<glm::quat>& orientations) {
						size_t index(0ull);
						for each (const auto & componentHandle in m_uuids) {
							if (auto* component = m_ecsWorld.getComponent<Transform_Component>(componentHandle)) {
								component->m_localTransform.m_orientation = orientations[index++];
								component->m_localTransform.update();
							}
						}
					}
					virtual void execute() override final {
						setOrientation(m_newData);
					}
					virtual void undo() override final {
						setOrientation(m_oldData);
					}
					virtual bool join(Editor_Command* const other) override final {
						if (auto newCommand = dynamic_cast<Rotate_Command*>(other)) {
							if (std::equal(m_uuids.cbegin(), m_uuids.cend(), newCommand->m_uuids.cbegin())) {
								m_newData = newCommand->m_newData;
								return true;
							}
						}
						return false;
					}
				};
				m_editor->doReversableAction(std::make_shared<Rotate_Command>(m_editor->getWorld(), getUUIDS(), glm::quat(glm::radians(rotInput))));
			}

			// Scaling
			auto& sclInput = transComponent->m_localTransform.m_scale;
			if (ImGui::DragFloat3("Scale", glm::value_ptr(sclInput))) {
				struct Scale_Command final : Editor_Command {
					ecsWorld& m_ecsWorld;
					const std::vector<ComponentHandle> m_uuids;
					std::vector<glm::vec3> m_oldData, m_newData;
					Scale_Command(ecsWorld& world, const std::vector<ComponentHandle>& uuids, const glm::vec3& newScale)
						: m_ecsWorld(world), m_uuids(uuids) {
						for each (const auto & componentHandle in m_uuids) {
							if (const auto* component = m_ecsWorld.getComponent<Transform_Component>(componentHandle)) {
								m_oldData.push_back(component->m_localTransform.m_scale);
								m_newData.push_back(newScale);
							}
						}
					}
					void setScale(const std::vector<glm::vec3>& scales) {
						if (scales.size()) {
							size_t index(0ull);
							for each (const auto & componentHandle in m_uuids) {
								if (auto* component = m_ecsWorld.getComponent<Transform_Component>(componentHandle)) {
									component->m_localTransform.m_scale = scales[index++];
									component->m_localTransform.update();
								}
							}
						}
					}
					virtual void execute() override final {
						setScale(m_newData);
					}
					virtual void undo() override final {
						setScale(m_oldData);
					}
					virtual bool join(Editor_Command* const other) override final {
						if (auto newCommand = dynamic_cast<Scale_Command*>(other)) {
							if (std::equal(m_uuids.cbegin(), m_uuids.cend(), newCommand->m_uuids.cbegin())) {
								m_newData = newCommand->m_newData;
								return true;
							}
						}
						return false;
					}
				};
				m_editor->doReversableAction(std::make_shared<Scale_Command>(m_editor->getWorld(), getUUIDS(), sclInput));
			}
		}
	}


private:
	// Private Attributes
	Engine* m_engine = nullptr;
	LevelEditor_Module* m_editor = nullptr;
};

#endif // INSPECTOR_TRANSFORM_SYSTEM_H