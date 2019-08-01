#pragma once
#ifndef INSPECTOR_LIGHTRADIUS_SYSTEM_H
#define INSPECTOR_LIGHTRADIUS_SYSTEM_H 

#include "Modules/Editor/Editor_M.h"
#include "Modules/World/ECS/ecsSystem.h"
#include "Modules/World/ECS/components.h"
#include "Modules/UI/dear imgui/imgui.h"
#include "glm/gtc/type_ptr.hpp"


/***/
class Inspector_LightRadius_System : public BaseECSSystem {
public:
	// Public (de)Constructors
	/***/
	inline Inspector_LightRadius_System(LevelEditor_Module* editor)
		: m_editor(editor) {
		// Declare component types used
		addComponentType(LightRadius_Component::ID);
	}


	// Public Interface Implementation
	inline virtual void updateComponents(const float& deltaTime, const std::vector< std::vector<BaseECSComponent*> >& components) override {
		const auto& selectedEntities = m_editor->getSelection();
		std::vector<LightRadius_Component*> selectedComponents;
		for each (const auto & componentParam in components) {
			auto* component = (LightRadius_Component*)componentParam[0];
			if (std::find(selectedEntities.cbegin(), selectedEntities.cend(), component->entity) != selectedEntities.cend())
				selectedComponents.push_back(component);
		}
		if (selectedComponents.size()) {
			static bool open = true;
			const auto text = LightRadius_Component::NAME + "(" + std::to_string(selectedComponents.size()) + ")";
			if (ImGui::CollapsingHeader(text.c_str(), &open, ImGuiTreeNodeFlags_DefaultOpen)) {
				auto radiusInput = selectedComponents[0]->m_radius;
				if (ImGui::DragFloat("Radius", &radiusInput))
					for each (auto & component in selectedComponents)
						component->m_radius = radiusInput;
			}
		}
	}


private:
	// Private Attributes
	LevelEditor_Module* m_editor = nullptr;
};
#endif // INSPECTOR_LIGHTRADIUS_SYSTEM_H