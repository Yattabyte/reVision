#pragma once
#ifndef INSPECTOR_LIGHT_SYSTEM_H
#define INSPECTOR_LIGHT_SYSTEM_H

#include "Modules/Editor/Editor_M.h"
#include "Modules/ECS/ecsSystem.h"
#include "Modules/ECS/component_types.h"
#include "imgui.h"
#include "glm/gtc/type_ptr.hpp"
#include "Engine.h"


/** An ECS system allowing the user to inspect selected light components. */
class Inspector_Light_System final : public ecsBaseSystem {
public:
	// Public (De)Constructors
	/** Destroy this system. */
	inline ~Inspector_Light_System() = default;
	/** Construct this system. */
	inline Inspector_Light_System(Engine* engine, LevelEditor_Module* editor) noexcept :
		m_engine(engine),
		m_editor(editor)
	{
		// Declare component types used
		addComponentType(Selected_Component::Runtime_ID);
		addComponentType(Light_Component::Runtime_ID);
	}


	// Public Interface Implementation
	inline virtual void updateComponents(const float& deltaTime, const std::vector<std::vector<ecsBaseComponent*>>& components) noexcept override final {
		const auto text = std::string(Light_Component::Name) + ": (" + std::to_string(components.size()) + ")";
		if (ImGui::CollapsingHeader(text.c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
			// Create list of handles for commands to use
			const auto getUUIDS = [&]() {
				std::vector<ComponentHandle> uuids;
				uuids.reserve(components.size());
				for each (const auto & componentParam in components)
					uuids.push_back(componentParam[1]->m_handle);
				return uuids;
			};
			auto* lightComponent = static_cast<Light_Component*>(components[0][1]);


			auto typeInput = lightComponent->m_type;
			constexpr const char* inputTypes[3] = {
				"Directional Light", "Point Light", "Spot Light"
			};
			static int item_current = static_cast<int>(typeInput);
			if (ImGui::Combo("Type", &item_current, inputTypes, IM_ARRAYSIZE(inputTypes))) {
				struct Type_Command final : Editor_Command {
					ecsWorld& m_ecsWorld;
					const std::vector<ComponentHandle> m_uuids;
					std::vector<Light_Component::Light_Type> m_oldData, m_newData;
					Type_Command(ecsWorld& world, const std::vector<ComponentHandle>& uuids, const Light_Component::Light_Type& data) noexcept
						: m_ecsWorld(world), m_uuids(uuids) {
						for each (const auto & componentHandle in m_uuids) {
							if (const auto* component = m_ecsWorld.getComponent<Light_Component>(componentHandle)) {
								m_oldData.push_back(component->m_type);
								m_newData.push_back(data);
							}
						}
					}
					void setData(const std::vector<Light_Component::Light_Type>& data) noexcept {
						if (data.size()) {
							size_t index(0ull);
							for each (const auto & componentHandle in m_uuids)
								if (auto* component = m_ecsWorld.getComponent<Light_Component>(componentHandle))
									component->m_type = data[index++];
						}
					}
					virtual void execute() noexcept override final {
						setData(m_newData);
					}
					virtual void undo() noexcept override final {
						setData(m_oldData);
					}
					virtual bool join(Editor_Command* other) noexcept override final {
						if (auto newCommand = dynamic_cast<Type_Command*>(other)) {
							if (std::equal(m_uuids.cbegin(), m_uuids.cend(), newCommand->m_uuids.cbegin())) {
								m_newData = newCommand->m_newData;
								return true;
							}
						}
						return false;
					}
				};
				m_editor->doReversableAction(std::make_shared<Type_Command>(m_editor->getWorld(), getUUIDS(), static_cast<Light_Component::Light_Type>(item_current)));
			}

			auto colorInput = lightComponent->m_color;
			if (ImGui::ColorEdit3("Color", glm::value_ptr(colorInput))) {
				struct Color_Command final : Editor_Command {
					ecsWorld& m_ecsWorld;
					const std::vector<ComponentHandle> m_uuids;
					std::vector<glm::vec3> m_oldData, m_newData;
					Color_Command(ecsWorld& world, const std::vector<ComponentHandle>& uuids, const glm::vec3& data) noexcept
						: m_ecsWorld(world), m_uuids(uuids) {
						for each (const auto & componentHandle in m_uuids) {
							if (const auto* component = m_ecsWorld.getComponent<Light_Component>(componentHandle)) {
								m_oldData.push_back(component->m_color);
								m_newData.push_back(data);
							}
						}
					}
					void setData(const std::vector<glm::vec3>& data) noexcept {
						if (data.size()) {
							size_t index(0ull);
							for each (const auto & componentHandle in m_uuids)
								if (auto* component = m_ecsWorld.getComponent<Light_Component>(componentHandle))
									component->m_color = data[index++];
						}
					}
					virtual void execute() noexcept override final {
						setData(m_newData);
					}
					virtual void undo() noexcept override final {
						setData(m_oldData);
					}
					virtual bool join(Editor_Command* other) noexcept override final {
						if (auto newCommand = dynamic_cast<Color_Command*>(other)) {
							if (std::equal(m_uuids.cbegin(), m_uuids.cend(), newCommand->m_uuids.cbegin())) {
								m_newData = newCommand->m_newData;
								return true;
							}
						}
						return false;
					}
				};
				m_editor->doReversableAction(std::make_shared<Color_Command>(m_editor->getWorld(), getUUIDS(), colorInput));
			}

			auto intensityInput = lightComponent->m_intensity;
			if (ImGui::DragFloat("Intensity", &intensityInput)) {
				struct Intensity_Command final : Editor_Command {
					ecsWorld& m_ecsWorld;
					const std::vector<ComponentHandle> m_uuids;
					std::vector<float> m_oldData, m_newData;
					Intensity_Command(ecsWorld& world, const std::vector<ComponentHandle>& uuids, const float& data) noexcept
						: m_ecsWorld(world), m_uuids(uuids) {
						for each (const auto & componentHandle in m_uuids) {
							if (const auto* component = m_ecsWorld.getComponent<Light_Component>(componentHandle)) {
								m_oldData.push_back(component->m_intensity);
								m_newData.push_back(data);
							}
						}
					}
					void setData(const std::vector<float>& data) noexcept {
						if (data.size()) {
							size_t index(0ull);
							for each (const auto & componentHandle in m_uuids) {
								if (auto* component = m_ecsWorld.getComponent<Light_Component>(componentHandle))
									component->m_intensity = data[index++];
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
						if (auto newCommand = dynamic_cast<Intensity_Command*>(other)) {
							if (std::equal(m_uuids.cbegin(), m_uuids.cend(), newCommand->m_uuids.cbegin())) {
								m_newData = newCommand->m_newData;
								return true;
							}
						}
						return false;
					}
				};
				m_editor->doReversableAction(std::make_shared<Intensity_Command>(m_editor->getWorld(), getUUIDS(), intensityInput));
			}

			auto radiusInput = lightComponent->m_radius;
			if (ImGui::DragFloat("Radius", &radiusInput)) {
				struct Radius_Command final : Editor_Command {
					ecsWorld& m_ecsWorld;
					const std::vector<ComponentHandle> m_uuids;
					std::vector<float> m_oldData, m_newData;
					Radius_Command(ecsWorld& world, const std::vector<ComponentHandle>& uuids, const float& data) noexcept
						: m_ecsWorld(world), m_uuids(uuids) {
						for each (const auto & componentHandle in m_uuids) {
							if (const auto* component = m_ecsWorld.getComponent<Light_Component>(componentHandle)) {
								m_oldData.push_back(component->m_radius);
								m_newData.push_back(data);
							}
						}
					}
					void setData(const std::vector<float>& data) noexcept {
						if (data.size()) {
							size_t index(0ull);
							for each (const auto & componentHandle in m_uuids) {
								if (auto* component = m_ecsWorld.getComponent<Light_Component>(componentHandle))
									component->m_radius = data[index++];
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
						if (auto newCommand = dynamic_cast<Radius_Command*>(other)) {
							if (std::equal(m_uuids.cbegin(), m_uuids.cend(), newCommand->m_uuids.cbegin())) {
								m_newData = newCommand->m_newData;
								return true;
							}
						}
						return false;
					}
				};
				m_editor->doReversableAction(std::make_shared<Radius_Command>(m_editor->getWorld(), getUUIDS(), radiusInput));
			}

			auto cutoffInput = lightComponent->m_cutoff;
			if (ImGui::DragFloat("Cutoff", &cutoffInput)) {
				struct Cutoff_Command final : Editor_Command {
					ecsWorld& m_ecsWorld;
					const std::vector<ComponentHandle> m_uuids;
					std::vector<float> m_oldData, m_newData;
					Cutoff_Command(ecsWorld& world, const std::vector<ComponentHandle>& uuids, const float& data) noexcept
						: m_ecsWorld(world), m_uuids(uuids) {
						for each (const auto & componentHandle in m_uuids) {
							if (const auto* component = m_ecsWorld.getComponent<Light_Component>(componentHandle)) {
								m_oldData.push_back(component->m_cutoff);
								m_newData.push_back(data);
							}
						}
					}
					void setData(const std::vector<float>& data) noexcept {
						if (data.size()) {
							size_t index(0ull);
							for each (const auto & componentHandle in m_uuids) {
								if (auto* component = m_ecsWorld.getComponent<Light_Component>(componentHandle))
									component->m_cutoff = data[index++];
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
						if (auto newCommand = dynamic_cast<Cutoff_Command*>(other)) {
							if (std::equal(m_uuids.cbegin(), m_uuids.cend(), newCommand->m_uuids.cbegin())) {
								m_newData = newCommand->m_newData;
								return true;
							}
						}
						return false;
					}
				};
				m_editor->doReversableAction(std::make_shared<Cutoff_Command>(m_editor->getWorld(), getUUIDS(), cutoffInput));
			}
		}
	}


private:
	// Private Attributes
	Engine* m_engine = nullptr;
	LevelEditor_Module* m_editor = nullptr;
};

#endif // INSPECTOR_LIGHT_SYSTEM_H