#include "Modules/Editor/Systems/Inspector_Transform_System.h"
#include "Modules/Editor/Editor_M.h"
#include "Modules/ECS/component_types.h"
#include "Engine.h"
#include "imgui.h"


Inspector_Transform_System::Inspector_Transform_System(Engine& engine, LevelEditor_Module& editor) :
	m_engine(engine),
	m_editor(editor)
{
	// Declare component types used
	addComponentType(Selected_Component::Runtime_ID);
	addComponentType(Transform_Component::Runtime_ID);
}

void Inspector_Transform_System::updateComponents(const float&, const std::vector<std::vector<ecsBaseComponent*>>& components) 
{
	ImGui::PushID(this);
	const auto text = std::string(Transform_Component::Name) + ": (" + std::to_string(components.size()) + ")";
	if (ImGui::CollapsingHeader(text.c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
		// Create list of handles for commands to use
		const auto getUUIDS = [&] {
			std::vector<ComponentHandle> uuids;
			uuids.reserve(components.size());
			for (const auto& componentParam : components)
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
					for (const auto& componentHandle : m_uuids) {
						if (const auto* component = m_ecsWorld.getComponent<Transform_Component>(componentHandle)) {
							m_oldData.push_back(component->m_localTransform.m_position);
							m_newData.push_back(newPosition);
						}
					}
				}
				void setPosition(const std::vector<glm::vec3>& positions) {
					if (positions.size()) {
						size_t index(0ull);
						for (const auto& componentHandle : m_uuids) {
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
				void execute() final {
					setPosition(m_newData);
				}
				void undo() final {
					setPosition(m_oldData);
				}
				bool join(Editor_Command* other) final {
					if (const auto& newCommand = dynamic_cast<Move_Command*>(other)) {
						if (std::equal(m_uuids.cbegin(), m_uuids.cend(), newCommand->m_uuids.cbegin())) {
							m_newData = newCommand->m_newData;
							return true;
						}
					}
					return false;
				}
			};
			m_editor.doReversableAction(std::make_shared<Move_Command>(m_editor.getWorld(), m_editor, getUUIDS(), posInput));
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
					for (const auto& componentHandle : m_uuids) {
						if (const auto* component = m_ecsWorld.getComponent<Transform_Component>(componentHandle)) {
							m_oldData.push_back(component->m_localTransform.m_orientation);
							m_newData.push_back(newOrientation);
						}
					}
				}
				void setOrientation(const std::vector<glm::quat>& orientations) {
					size_t index(0ull);
					for (const auto& componentHandle : m_uuids) {
						if (auto* component = m_ecsWorld.getComponent<Transform_Component>(componentHandle)) {
							component->m_localTransform.m_orientation = orientations[index++];
							component->m_localTransform.update();
						}
					}
				}
				void execute() final {
					setOrientation(m_newData);
				}
				void undo() final {
					setOrientation(m_oldData);
				}
				bool join(Editor_Command* other) final {
					if (const auto& newCommand = dynamic_cast<Rotate_Command*>(other)) {
						if (std::equal(m_uuids.cbegin(), m_uuids.cend(), newCommand->m_uuids.cbegin())) {
							m_newData = newCommand->m_newData;
							return true;
						}
					}
					return false;
				}
			};
			m_editor.doReversableAction(std::make_shared<Rotate_Command>(m_editor.getWorld(), getUUIDS(), glm::quat(glm::radians(rotInput))));
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
					for (const auto& componentHandle : m_uuids) {
						if (const auto* component = m_ecsWorld.getComponent<Transform_Component>(componentHandle)) {
							m_oldData.push_back(component->m_localTransform.m_scale);
							m_newData.push_back(newScale);
						}
					}
				}
				void setScale(const std::vector<glm::vec3>& scales) {
					if (scales.size()) {
						size_t index(0ull);
						for (const auto& componentHandle : m_uuids) {
							if (auto* component = m_ecsWorld.getComponent<Transform_Component>(componentHandle)) {
								component->m_localTransform.m_scale = scales[index++];
								component->m_localTransform.update();
							}
						}
					}
				}
				void execute() final {
					setScale(m_newData);
				}
				void undo() final {
					setScale(m_oldData);
				}
				bool join(Editor_Command* other) final {
					if (const auto& newCommand = dynamic_cast<Scale_Command*>(other)) {
						if (std::equal(m_uuids.cbegin(), m_uuids.cend(), newCommand->m_uuids.cbegin())) {
							m_newData = newCommand->m_newData;
							return true;
						}
					}
					return false;
				}
			};
			m_editor.doReversableAction(std::make_shared<Scale_Command>(m_editor.getWorld(), getUUIDS(), sclInput));
		}
	}
	ImGui::PopID();
}