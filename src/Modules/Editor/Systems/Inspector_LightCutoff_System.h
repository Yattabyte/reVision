#pragma once
#ifndef INSPECTOR_LIGHTCUTOFF_SYSTEM_H
#define INSPECTOR_LIGHTCUTOFF_SYSTEM_H 

#include "Modules/Editor/Editor_M.h"
#include "Modules/World/ECS/ecsSystem.h"
#include "Modules/World/ECS/components.h"
#include "Modules/UI/dear imgui/imgui.h"
#include "glm/gtc/type_ptr.hpp"


/***/
class Inspector_LightCutoff_System : public BaseECSSystem {
public:
	// Public (de)Constructors
	/***/
	inline Inspector_LightCutoff_System(LevelEditor_Module* editor)
		: m_editor(editor) {
		// Declare component types used
		addComponentType(LightCutoff_Component::ID);
	}


	// Public Interface Implementation
	inline virtual void updateComponents(const float& deltaTime, const std::vector< std::vector<BaseECSComponent*> >& components) override {
		const auto& selectedEntities = m_editor->getSelection();
		std::vector<LightCutoff_Component*> selectedComponents;
		for each (const auto & componentParam in components) {
			auto* component = (LightCutoff_Component*)componentParam[0];
			if (std::find(selectedEntities.cbegin(), selectedEntities.cend(), component->entity) != selectedEntities.cend())
				selectedComponents.push_back(component);
		}
		if (selectedComponents.size()) {
			static bool open = true;
			const auto text = LightCutoff_Component::NAME + "(" + std::to_string(selectedComponents.size()) + ")";
			if (ImGui::CollapsingHeader(text.c_str(), &open, ImGuiTreeNodeFlags_DefaultOpen)) {
				auto cutoffInput = selectedComponents[0]->m_cutoff;
				if (ImGui::DragFloat("Cutoff", &cutoffInput))
					for each (auto & component in selectedComponents)
						component->m_cutoff = cutoffInput;
			}
		}
	}


private:
	// Private Attributes
	LevelEditor_Module* m_editor = nullptr;
};
#endif // INSPECTOR_LIGHTCUTOFF_SYSTEM_H