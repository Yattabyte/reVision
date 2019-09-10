#pragma once
#ifndef INSPECTOR_LIGHTCOLOR_SYSTEM_H
#define INSPECTOR_LIGHTCOLOR_SYSTEM_H 

#include "Modules/Editor/Editor_M.h"
#include "Modules/World/World_M.h"
#include "Modules/World/ECS/ecsSystem.h"
#include "Modules/World/ECS/components.h"
#include "Modules/UI/dear imgui/imgui.h"
#include "glm/gtc/type_ptr.hpp"
#include "Engine.h"


/** An ECS system allowing the user to inspect selected light color components. */
class Inspector_LightColor_System : public BaseECSSystem {
public:
	// Public (de)Constructors
	/** Destroy this system. */
	inline ~Inspector_LightColor_System() = default;
	/** Construct this system. */
	inline Inspector_LightColor_System(Engine* engine, LevelEditor_Module* editor)
		: m_engine(engine), m_editor(editor) {
		// Declare component types used
		addComponentType(Selected_Component::ID);
		addComponentType(LightColor_Component::ID);
	}


	// Public Interface Implementation
	inline virtual void updateComponents(const float& deltaTime, const std::vector< std::vector<BaseECSComponent*> >& components) override {
		const auto text = LightColor_Component::STRING_NAME + ": (" + std::to_string(components.size()) + ")";
		if (ImGui::CollapsingHeader(text.c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
			// Create list of handles for commands to use
			std::vector<ecsHandle> uuids;
			uuids.reserve(components.size());
			for each (const auto & componentParam in components)
				uuids.push_back(componentParam[0]->m_entity);

			auto colorInput = ((LightColor_Component*)components[0][1])->m_color;
			if (ImGui::ColorEdit3("Color", glm::value_ptr(colorInput)))
				for each (auto & componentParam in components)
					((LightColor_Component*)componentParam[1])->m_color = colorInput;
			auto intensityInput = ((LightColor_Component*)(components[0][1]))->m_intensity;
			if (ImGui::DragFloat("Intensity", &intensityInput))
				for each (auto & componentParam in components)
					((LightColor_Component*)componentParam[1])->m_intensity = intensityInput;
		}
	}


private:
	// Private Attributes
	Engine* m_engine = nullptr;
	LevelEditor_Module* m_editor = nullptr;
};
#endif // INSPECTOR_LIGHTCOLOR_SYSTEM_H