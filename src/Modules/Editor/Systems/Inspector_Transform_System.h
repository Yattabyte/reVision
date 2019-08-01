#pragma once
#ifndef INSPECTOR_TRANSFORM_SYSTEM_H
#define INSPECTOR_TRANSFORM_SYSTEM_H 

#include "Modules/Editor/Editor_M.h"
#include "Modules/World/ECS/ecsSystem.h"
#include "Modules/World/ECS/components.h"
#include "Modules/UI/dear imgui/imgui.h"
#include "glm/gtc/type_ptr.hpp"


/***/
class Inspector_Transform_System : public BaseECSSystem {
public:
	// Public (de)Constructors
	/***/
	inline Inspector_Transform_System(LevelEditor_Module* editor)
		: m_editor(editor) {
		// Declare component types used
		addComponentType(Transform_Component::ID);
	}


	// Public Interface Implementation
	inline virtual void updateComponents(const float& deltaTime, const std::vector< std::vector<BaseECSComponent*> >& components) override {
		const auto & selectedEntities = m_editor->getSelection();
		std::vector<Transform_Component*> selectedComponents;
		for each (const auto & componentParam in components) {
			auto* component = (Transform_Component*)componentParam[0];
			if (std::find(selectedEntities.cbegin(), selectedEntities.cend(), component->entity) != selectedEntities.cend())
				selectedComponents.push_back(component);
		}
		if (selectedComponents.size()) {
			static bool open = true;
			const auto text = Transform_Component::NAME + "(" + std::to_string(selectedComponents.size()) + ")";
			if (ImGui::CollapsingHeader(text.c_str(), &open, ImGuiTreeNodeFlags_DefaultOpen)) {
				auto posInput = selectedComponents[0]->m_transform.m_position;
				if (ImGui::DragFloat3("Position", glm::value_ptr(posInput))) {
					for each (auto & component in selectedComponents) {
						component->m_transform.m_position = posInput;
						component->m_transform.update();
					}
				}
				auto sclInput = selectedComponents[0]->m_transform.m_scale;
				if (ImGui::DragFloat3("Scale", glm::value_ptr(sclInput))) {
					for each (auto & component in selectedComponents) {
						component->m_transform.m_scale = sclInput;
						component->m_transform.update();
					}
				}
				auto rotInput = selectedComponents[0]->m_transform.m_orientation;
				if (ImGui::DragFloat4("Orientation", glm::value_ptr(rotInput))) {
					for each (auto & component in selectedComponents) {
						component->m_transform.m_orientation = rotInput;
						component->m_transform.update();
					}
				}
			}
		}
	}


private:
	// Private Attributes
	LevelEditor_Module* m_editor = nullptr;
};
#endif // INSPECTOR_TRANSFORM_SYSTEM_H