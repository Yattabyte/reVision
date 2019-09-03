#pragma once
#ifndef INSPECTOR_LIGHTCUTOFF_SYSTEM_H
#define INSPECTOR_LIGHTCUTOFF_SYSTEM_H 

#include "Modules/World/ECS/ecsSystem.h"
#include "Modules/World/ECS/components.h"
#include "Modules/UI/dear imgui/imgui.h"
#include "glm/gtc/type_ptr.hpp"


/** An ECS system allowing the user to inspect selected light cutoff components. */
class Inspector_LightCutoff_System : public BaseECSSystem {
public:
	// Public (de)Constructors
	/** Destroy this system. */
	inline ~Inspector_LightCutoff_System() = default;
	/** Construct this system. */
	inline Inspector_LightCutoff_System() {
		// Declare component types used
		addComponentType(Selected_Component::ID);
		addComponentType(LightCutoff_Component::ID);
	}


	// Public Interface Implementation
	inline virtual void updateComponents(const float& deltaTime, const std::vector< std::vector<BaseECSComponent*> >& components) override {
		const auto text = LightCutoff_Component::STRING_NAME + ": (" + std::to_string(components.size()) + ")";
		if (ImGui::CollapsingHeader(text.c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
			auto cutoffInput = ((LightCutoff_Component*)components[0][1])->m_cutoff;
			if (ImGui::DragFloat("Cutoff", &cutoffInput))
				for each (auto & componentParam in components)
					((LightCutoff_Component*)componentParam[1])->m_cutoff = cutoffInput;
		}
	}
};
#endif // INSPECTOR_LIGHTCUTOFF_SYSTEM_H