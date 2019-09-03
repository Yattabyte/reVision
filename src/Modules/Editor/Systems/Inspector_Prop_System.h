#pragma once
#ifndef INSPECTOR_PROP_SYSTEM_H
#define INSPECTOR_PROP_SYSTEM_H 

#include "Modules/World/ECS/ecsSystem.h"
#include "Modules/World/ECS/components.h"
#include "Modules/UI/dear imgui/imgui.h"
#include "glm/gtc/type_ptr.hpp"


/***/
class Inspector_Prop_System : public BaseECSSystem {
public:
	// Public (de)Constructors
	/***/
	inline Inspector_Prop_System() {
		// Declare component types used
		addComponentType(Selected_Component::ID);
		addComponentType(Prop_Component::ID);
	}


	// Public Interface Implementation
	inline virtual void updateComponents(const float& deltaTime, const std::vector< std::vector<BaseECSComponent*> >& components) override {
		const auto text = Prop_Component::STRING_NAME + ": (" + std::to_string(components.size()) + ")";
		if (ImGui::CollapsingHeader(text.c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
			char nameInput[256];
			for (size_t x = 0; x < ((Prop_Component*)components[0][1])->m_modelName.length() && x < IM_ARRAYSIZE(nameInput); ++x)
				nameInput[x] = ((Prop_Component*)components[0][1])->m_modelName[x];
			nameInput[std::min(256ull, ((Prop_Component*)components[0][1])->m_modelName.length())] = '\0';
			if (ImGui::InputText("Model Name", nameInput, IM_ARRAYSIZE(nameInput)))
				for each (auto & componentParam in components) {
					auto* component = ((Prop_Component*)componentParam[1]);
					component->m_modelName = nameInput;
					component->m_model.reset();
					component->m_uploadModel = false;
					component->m_uploadMaterial = false;
					component->m_offset = 0ull;
					component->m_count = 0ull;
					component->m_materialID = 0u;
				}
			auto skinInput = (int)((Prop_Component*)components[0][1])->m_skin;
			if (ImGui::DragInt("Skin", &skinInput))
				for each (auto & componentParam in components)
					((Prop_Component*)componentParam[1])->m_skin = (unsigned int)skinInput;
		}
	}
};
#endif // INSPECTOR_PROP_SYSTEM_H