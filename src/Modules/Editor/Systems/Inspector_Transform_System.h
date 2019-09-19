#pragma once
#ifndef INSPECTOR_TRANSFORM_SYSTEM_H
#define INSPECTOR_TRANSFORM_SYSTEM_H 

#include "Modules/Editor/Editor_M.h"
#include "Modules/World/World_M.h"
#include "Modules/World/ECS/ecsSystem.h"
#include "Modules/World/ECS/components.h"
#include "Modules/UI/dear imgui/imgui.h"
#include "glm/gtc/type_ptr.hpp"
#include "Engine.h"


/** An ECS system allowing the user to inspect selected component transforms.*/
class Inspector_Transform_System : public BaseECSSystem {
public:
	// Public (de)Constructors
	/** Destroy this system. */
	inline ~Inspector_Transform_System() = default;
	/** Construct this system. 
	@param	editor		the level editor. */
	inline Inspector_Transform_System(Engine* engine, LevelEditor_Module* editor)
		: m_engine(engine), m_editor(editor) {
		// Declare component types used
		addComponentType(Selected_Component::ID);
		addComponentType(Transform_Component::ID);
	}


	// Public Interface Implementation
	inline virtual void updateComponents(const float& deltaTime, const std::vector< std::vector<BaseECSComponent*> >& components) override {		
		const auto text = Transform_Component::STRING_NAME + ": (" + std::to_string(components.size()) + ")";
		if (ImGui::CollapsingHeader(text.c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
			// Create list of handles for commands to use
			const auto getUUIDS = [&]() {
				std::vector<ecsHandle> uuids;
				uuids.reserve(components.size());
				for each (const auto & componentParam in components)
					uuids.push_back(componentParam[0]->m_entity);
				return uuids;
			};

			// Position
			auto posInput = ((Transform_Component*)components[0][1])->m_localTransform.m_position;
			if (ImGui::DragFloat3("Position", glm::value_ptr(posInput))) {
				struct Move_Command : Editor_Command {
					World_Module& m_world;
					LevelEditor_Module& m_editor;
					const std::vector<ecsHandle> m_uuids;
					std::vector<glm::vec3> m_oldData, m_newData;
					Move_Command(World_Module& world, LevelEditor_Module& editor, const std::vector<ecsHandle>& uuids, const glm::vec3& newPosition)
						: m_world(world), m_editor(editor), m_uuids(uuids) {
						for each (const auto & entityHandle in m_uuids) {
							const auto* component = m_world.getComponent<Transform_Component>(entityHandle);
							m_oldData.push_back(component->m_localTransform.m_position);
							m_newData.push_back(newPosition);
						}
					}
					void setPosition(const std::vector<glm::vec3>& positions) {
						if (positions.size()) {
							size_t index(0ull);
							for each (const auto & entityHandle in m_uuids) {
								auto* component = m_world.getComponent<Transform_Component>(entityHandle);
								component->m_localTransform.m_position = positions[index++];
								component->m_localTransform.update();
							}
							auto newTransform = m_editor.getGizmoTransform();
							newTransform.m_position = positions[0];
							newTransform.update();
							m_editor.setGizmoTransform(newTransform);
						}
					}
					virtual void execute() {
						setPosition(m_newData);
					}
					virtual void undo() {
						setPosition(m_oldData);
					}
					virtual bool join(Editor_Command* const other) {
						if (auto newCommand = dynamic_cast<Move_Command*>(other)) {
							if (std::equal(m_uuids.cbegin(), m_uuids.cend(), newCommand->m_uuids.cbegin())) {
								m_newData = newCommand->m_newData;
								return true;
							}
						}
						return false;
					}
				};
				m_editor->doReversableAction(std::make_shared<Move_Command>(m_engine->getModule_World(), *m_editor, getUUIDS(), posInput));
			}

			// Rotation
			auto rotInput = glm::degrees(glm::eulerAngles(((Transform_Component*)components[0][1])->m_localTransform.m_orientation));
			if (ImGui::DragFloat3("Rotation", glm::value_ptr(rotInput))) {
				struct Rotate_Command : Editor_Command {
					World_Module& m_world;
					const std::vector<ecsHandle> m_uuids;
					std::vector<glm::quat> m_oldData, m_newData;
					Rotate_Command(World_Module& world, const std::vector<ecsHandle>& uuids, const glm::quat& newOrientation)
						: m_world(world), m_uuids(uuids) {
						for each (const auto & entityHandle in m_uuids) {
							const auto* component = m_world.getComponent<Transform_Component>(entityHandle);
							m_oldData.push_back(component->m_localTransform.m_orientation);
							m_newData.push_back(newOrientation);
						}
					}
					void setOrientation(const std::vector<glm::quat>& orientations) {
						size_t index(0ull);
						for each (const auto & entityHandle in m_uuids) {
							auto* component = m_world.getComponent<Transform_Component>(entityHandle);
							component->m_localTransform.m_orientation = orientations[index++];
							component->m_localTransform.update();
						}
					}
					virtual void execute() {
						setOrientation(m_newData);
					}
					virtual void undo() {
						setOrientation(m_oldData);
					}
					virtual bool join(Editor_Command* const other) {
						if (auto newCommand = dynamic_cast<Rotate_Command*>(other)) {
							if (std::equal(m_uuids.cbegin(), m_uuids.cend(), newCommand->m_uuids.cbegin())) {
								m_newData = newCommand->m_newData;
								return true;
							}
						}
						return false;
					}
				};
				m_editor->doReversableAction(std::make_shared<Rotate_Command>(m_engine->getModule_World(), getUUIDS(), glm::quat(glm::radians(rotInput))));
			}

			// Sclaing
			auto sclInput = ((Transform_Component*)components[0][1])->m_localTransform.m_scale;
			if (ImGui::DragFloat3("Scale", glm::value_ptr(sclInput))) {
				struct Scale_Command : Editor_Command {
					World_Module& m_world;
					const std::vector<ecsHandle> m_uuids;
					std::vector<glm::vec3> m_oldData, m_newData;
					Scale_Command(World_Module& world, const std::vector<ecsHandle>& uuids, const glm::vec3& newScale)
						: m_world(world), m_uuids(uuids) {
						for each (const auto & entityHandle in m_uuids) {
							const auto* component = m_world.getComponent<Transform_Component>(entityHandle);
							m_oldData.push_back(component->m_localTransform.m_scale);
							m_newData.push_back(newScale);
						}
					}
					void setScale(const std::vector<glm::vec3>& scales) {
						if (scales.size()) {
							size_t index(0ull);
							for each (const auto & entityHandle in m_uuids) {
								auto* component = m_world.getComponent<Transform_Component>(entityHandle);
								component->m_localTransform.m_scale = scales[index++];
								component->m_localTransform.update();
							}
						}
					}
					virtual void execute() {
						setScale(m_newData);
					}
					virtual void undo() {
						setScale(m_oldData);
					}
					virtual bool join(Editor_Command* const other) {
						if (auto newCommand = dynamic_cast<Scale_Command*>(other)) {
							if (std::equal(m_uuids.cbegin(), m_uuids.cend(), newCommand->m_uuids.cbegin())) {
								m_newData = newCommand->m_newData;
								return true;
							}
						}
						return false;
					}
				};
				m_editor->doReversableAction(std::make_shared<Scale_Command>(m_engine->getModule_World(), getUUIDS(), sclInput));
			}
		}
	}


private:
	// Private Attributes
	Engine* m_engine = nullptr;
	LevelEditor_Module* m_editor = nullptr;
};
#endif // INSPECTOR_TRANSFORM_SYSTEM_H