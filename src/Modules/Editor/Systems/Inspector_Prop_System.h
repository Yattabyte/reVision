#pragma once
#ifndef INSPECTOR_PROP_SYSTEM_H
#define INSPECTOR_PROP_SYSTEM_H 

#include "Modules/Editor/Editor_M.h"
#include "Modules/World/World_M.h"
#include "Modules/World/ECS/ecsSystem.h"
#include "Modules/World/ECS/components.h"
#include "Modules/UI/dear imgui/imgui.h"
#include "glm/gtc/type_ptr.hpp"
#include "Engine.h"


/** An ECS system allowing the user to inpsect selected prop components. */
class Inspector_Prop_System : public BaseECSSystem {
public:
	// Public (de)Constructors
	/** Destroy this system. */
	inline ~Inspector_Prop_System() = default;
	/** Construct this system. */
	inline Inspector_Prop_System(Engine* engine, LevelEditor_Module* editor)
		: m_engine(engine), m_editor(editor) {
		// Declare component types used
		addComponentType(Selected_Component::ID);
		addComponentType(Prop_Component::ID);
	}


	// Public Interface Implementation
	inline virtual void updateComponents(const float& deltaTime, const std::vector< std::vector<BaseECSComponent*> >& components) override {
		const auto text = Prop_Component::STRING_NAME + ": (" + std::to_string(components.size()) + ")";
		if (ImGui::CollapsingHeader(text.c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
			// Create list of handles for commands to use
			std::vector<ecsHandle> uuids;
			uuids.reserve(components.size());
			for each (const auto & componentParam in components)
				uuids.push_back(componentParam[0]->m_entity);

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


private:
	// Private Attributes
	Engine* m_engine = nullptr;
	LevelEditor_Module* m_editor = nullptr;
};
#endif // INSPECTOR_PROP_SYSTEM_H