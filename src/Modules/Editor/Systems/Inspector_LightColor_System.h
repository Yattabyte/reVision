#pragma once
#ifndef INSPECTOR_LIGHTCOLOR_SYSTEM_H
#define INSPECTOR_LIGHTCOLOR_SYSTEM_H 

#include "Modules/World/ECS/ecsSystem.h"
#include "Modules/World/ECS/components.h"
#include "Modules/UI/dear imgui/imgui.h"
#include "glm/gtc/type_ptr.hpp"


/** An ECS system allowing the user to inspect selected light color components. */
class Inspector_LightColor_System : public BaseECSSystem {
public:
	// Public (de)Constructors
	/** Destroy this system. */
	inline ~Inspector_LightColor_System() = default;
	/** Construct this system. */
	inline Inspector_LightColor_System() {
		// Declare component types used
		addComponentType(Selected_Component::ID);
		addComponentType(LightColor_Component::ID);
	}


	// Public Interface Implementation
	inline virtual void updateComponents(const float& deltaTime, const std::vector< std::vector<BaseECSComponent*> >& components) override {
		const auto text = LightColor_Component::STRING_NAME + ": (" + std::to_string(components.size()) + ")";
		if (ImGui::CollapsingHeader(text.c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
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
};
#endif // INSPECTOR_LIGHTCOLOR_SYSTEM_H