#pragma once
#ifndef INSPECTOR_LIGHTCOLOR_SYSTEM_H
#define INSPECTOR_LIGHTCOLOR_SYSTEM_H 

#include "Modules/Editor/Editor_M.h"
#include "Modules/World/ECS/ecsSystem.h"
#include "Modules/World/ECS/components.h"
#include "Modules/UI/dear imgui/imgui.h"
#include "glm/gtc/type_ptr.hpp"


/***/
class Inspector_LightColor_System : public BaseECSSystem {
public:
	// Public (de)Constructors
	/***/
	inline Inspector_LightColor_System(LevelEditor_Module * editor)
		: m_editor(editor) {
		// Declare component types used
		addComponentType(LightColor_Component::ID);
	}


	// Public Interface Implementation
	inline virtual void updateComponents(const float& deltaTime, const std::vector< std::vector<BaseECSComponent*> >& components) override {
		const auto& selectedEntities = m_editor->getSelection();
		std::vector<LightColor_Component*> selectedComponents;
		for each (const auto & componentParam in components) {
			auto* component = (LightColor_Component*)componentParam[0];
			if (std::find(selectedEntities.cbegin(), selectedEntities.cend(), component->entity) != selectedEntities.cend())
				selectedComponents.push_back(component);
		}
		if (selectedComponents.size()) {
			static bool open = true;
			const auto text = LightColor_Component::NAME + "(" + std::to_string(selectedComponents.size()) + ")";
			if (ImGui::CollapsingHeader(text.c_str(), &open, ImGuiTreeNodeFlags_DefaultOpen)) {
				auto colorInput = selectedComponents[0]->m_color;
				if (ImGui::ColorEdit3("Color", glm::value_ptr(colorInput)))
					for each (auto & component in selectedComponents)
						component->m_color = colorInput;
				auto intensityInput = selectedComponents[0]->m_intensity;
				if (ImGui::DragFloat("Intensity", &intensityInput))
					for each (auto & component in selectedComponents)
						component->m_intensity = intensityInput;
			}
		}
	}


private:
	// Private Attributes
	LevelEditor_Module * m_editor = nullptr;
};
#endif // INSPECTOR_LIGHTCOLOR_SYSTEM_H