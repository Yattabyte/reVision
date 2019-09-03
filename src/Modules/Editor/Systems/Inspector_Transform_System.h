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
		addComponentType(Selected_Component::ID);
		addComponentType(Transform_Component::ID);
	}


	// Public Interface Implementation
	inline virtual void updateComponents(const float& deltaTime, const std::vector< std::vector<BaseECSComponent*> >& components) override {		
		const auto text = Transform_Component::STRING_NAME + ": (" + std::to_string(components.size()) + ")";
		if (ImGui::CollapsingHeader(text.c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
			auto transform = m_editor->getGizmoTransform();
			bool changed = false;

			// Position
			auto posInput = ((Transform_Component*)components[0][1])->m_localTransform.m_position;
			if (ImGui::DragFloat3("Position", glm::value_ptr(posInput))) {
				changed = true;
				transform.m_position = posInput;
				transform.update();
			}

			// Rotation
			auto rotInput = glm::degrees(glm::eulerAngles(((Transform_Component*)components[0][1])->m_localTransform.m_orientation));
			if (ImGui::DragFloat3("Rotation", glm::value_ptr(rotInput))) {
				changed = true;
				transform.m_orientation = glm::quat(glm::radians(rotInput));
				transform.update();
			}

			// Sclaing
			auto sclInput = ((Transform_Component*)components[0][1])->m_localTransform.m_scale;
			if (ImGui::DragFloat3("Scale", glm::value_ptr(sclInput))) {
				changed = true;
				transform.m_scale = sclInput;
				transform.update();
			}

			// Update Components
			if (changed) {
				for each (auto & componentParam in components)
					((Transform_Component*)componentParam[1])->m_localTransform = transform;
				m_editor->setGizmoTransform(transform);
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