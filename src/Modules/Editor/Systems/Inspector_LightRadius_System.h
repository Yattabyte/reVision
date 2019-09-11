#pragma once
#ifndef INSPECTOR_LIGHTRADIUS_SYSTEM_H
#define INSPECTOR_LIGHTRADIUS_SYSTEM_H 

#include "Modules/Editor/Editor_M.h"
#include "Modules/World/World_M.h"
#include "Modules/World/ECS/ecsSystem.h"
#include "Modules/World/ECS/components.h"
#include "Modules/UI/dear imgui/imgui.h"
#include "glm/gtc/type_ptr.hpp"
#include "Engine.h"


/** An ECS system allowing the user to inpect selected light radius components. */
class Inspector_LightRadius_System : public BaseECSSystem {
public:
	// Public (de)Constructors
	/** Destroy this system. */
	inline ~Inspector_LightRadius_System() = default;
	/** Construct this system. */
	inline Inspector_LightRadius_System(Engine* engine, LevelEditor_Module* editor)
		: m_engine(engine), m_editor(editor) {
		// Declare component types used
		addComponentType(Selected_Component::ID);
		addComponentType(LightRadius_Component::ID);
	}


	// Public Interface Implementation
	inline virtual void updateComponents(const float& deltaTime, const std::vector< std::vector<BaseECSComponent*> >& components) override {
		const auto text = LightRadius_Component::STRING_NAME + ": (" + std::to_string(components.size()) + ")";
		if (ImGui::CollapsingHeader(text.c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
			// Create list of handles for commands to use
			std::vector<ecsHandle> uuids;
			uuids.reserve(components.size());
			for each (const auto & componentParam in components)
				uuids.push_back(componentParam[0]->m_entity);

			auto radiusInput = ((LightRadius_Component*)components[0][1])->m_radius;
			if (ImGui::DragFloat("Radius", &radiusInput)) {
				struct Radius_Command : Editor_Command {
					World_Module& m_world;
					std::vector<ecsHandle> m_uuids;
					std::vector<float> m_oldData, m_newData;
					Radius_Command(World_Module& world, const std::vector<ecsHandle>& uuids, const float& data)
						: m_world(world), m_uuids(uuids) {
						for each (const auto & entityHandle in m_uuids) {
							const auto* component = m_world.getComponent<LightRadius_Component>(entityHandle);
							m_oldData.push_back(component->m_radius);
							m_newData.push_back(data);
						}
					}
					void setData(const std::vector<float>& data) {
						if (data.size()) {
							size_t index(0ull);
							for each (const auto & entityHandle in m_uuids) {
								auto* component = m_world.getComponent<LightRadius_Component>(entityHandle);
								component->m_radius = data[index++];
							}
						}
					}
					virtual void execute() {
						setData(m_newData);
					}
					virtual void undo() {
						setData(m_oldData);
					}
				};
				m_editor->doReversableAction(std::make_shared<Radius_Command>(m_engine->getModule_World(), uuids, radiusInput));
			}
		}
	}


private:
	// Private Attributes
	Engine* m_engine = nullptr;
	LevelEditor_Module* m_editor = nullptr;
};
#endif // INSPECTOR_LIGHTRADIUS_SYSTEM_H