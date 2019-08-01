#pragma once
#ifndef INSPECTOR_PROP_SYSTEM_H
#define INSPECTOR_PROP_SYSTEM_H 

#include "Modules/Editor/Editor_M.h"
#include "Modules/World/ECS/ecsSystem.h"
#include "Modules/World/ECS/components.h"
#include "Modules/UI/dear imgui/imgui.h"
#include "glm/gtc/type_ptr.hpp"


/***/
class Inspector_Prop_System : public BaseECSSystem {
public:
	// Public (de)Constructors
	/***/
	inline Inspector_Prop_System(LevelEditor_Module* editor)
		: m_editor(editor) {
		// Declare component types used
		addComponentType(Prop_Component::ID);
	}


	// Public Interface Implementation
	inline virtual void updateComponents(const float& deltaTime, const std::vector< std::vector<BaseECSComponent*> >& components) override {
		const auto& selectedEntities = m_editor->getSelection();
		std::vector<Prop_Component*> selectedComponents;
		for each (const auto & componentParam in components) {
			auto* component = (Prop_Component*)componentParam[0];
			if (std::find(selectedEntities.cbegin(), selectedEntities.cend(), component->entity) != selectedEntities.cend())
				selectedComponents.push_back(component);
		}
		if (selectedComponents.size()) {
			static bool open = true;
			const auto text = Prop_Component::NAME + "(" + std::to_string(selectedComponents.size()) + ")";
			if (ImGui::CollapsingHeader(text.c_str(), &open, ImGuiTreeNodeFlags_DefaultOpen)) {
				char nameInput[256];
				for (size_t x = 0; x < selectedComponents[0]->m_modelName.length() && x < IM_ARRAYSIZE(nameInput); ++x)
					nameInput[x] = selectedComponents[0]->m_modelName[x];
				nameInput[std::min(256ull, selectedComponents[0]->m_modelName.length())] = '\0';
				if (ImGui::InputText("Model Name", nameInput, IM_ARRAYSIZE(nameInput)))
					for each (auto & component in selectedComponents) {
						component->m_modelName = nameInput;
						component->m_model.reset();
						component->m_uploadModel = false;
						component->m_uploadMaterial = false;
						component->m_offset = 0ull;
						component->m_count = 0ull;
						component->m_materialID = 0u;
					}
				auto skinInput = (int)selectedComponents[0]->m_skin;
				if (ImGui::DragInt("Skin", &skinInput))
					for each (auto & component in selectedComponents)
						component->m_skin = (unsigned int)skinInput;
			}
		}
	}


private:
	// Private Attributes
	LevelEditor_Module* m_editor = nullptr;
};
#endif // INSPECTOR_PROP_SYSTEM_H