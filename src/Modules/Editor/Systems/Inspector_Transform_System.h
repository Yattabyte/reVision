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
			static void* previousSelection = nullptr, * currentSelection = selectedComponents[0];
			static bool selectionChanged = false;
			if (previousSelection != currentSelection) {
				previousSelection = currentSelection;
				selectionChanged = true;
			}
			const auto text = Transform_Component::STRING_NAME + ": (" + std::to_string(selectedComponents.size()) + ")";
			if (ImGui::CollapsingHeader(text.c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {		
				auto transform = m_editor->getGizmoTransform();
				auto posInput = transform.m_position;
				if (ImGui::DragFloat3("Position", glm::value_ptr(posInput)))
					m_editor->moveSelection(posInput);				
				auto sclInput = transform.m_scale;
				if (ImGui::DragFloat3("Scale", glm::value_ptr(sclInput)))
					m_editor->scaleSelection(sclInput);				
				auto rotInput = glm::degrees(glm::eulerAngles(transform.m_orientation));
				if (ImGui::DragFloat3("Rotation", glm::value_ptr(rotInput)))
					m_editor->rotateSelection(glm::quat(glm::radians(rotInput)));
			}
		}
	}


private:
	// Private Methods
	/***/
	static Transform find_center_transform(const std::vector<Transform_Component*> & transforms) {
		Transform center;
		for each (const auto & component in transforms)
			center *= component->m_localTransform;
		center.m_position /= transforms.size();
		center.update();
		return center;
	}


	// Private Attributes
	LevelEditor_Module* m_editor = nullptr;
};
#endif // INSPECTOR_TRANSFORM_SYSTEM_H