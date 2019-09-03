#pragma once
#ifndef INSPECTOR_LIGHTRADIUS_SYSTEM_H
#define INSPECTOR_LIGHTRADIUS_SYSTEM_H 

#include "Modules/World/ECS/ecsSystem.h"
#include "Modules/World/ECS/components.h"
#include "Modules/UI/dear imgui/imgui.h"
#include "glm/gtc/type_ptr.hpp"


/** An ECS system allowing the user to inpect selected light radius components. */
class Inspector_LightRadius_System : public BaseECSSystem {
public:
	// Public (de)Constructors
	/** Destroy this system. */
	inline ~Inspector_LightRadius_System() = default;
	/** Construct this system. */
	inline Inspector_LightRadius_System() {
		// Declare component types used
		addComponentType(Selected_Component::ID);
		addComponentType(LightRadius_Component::ID);
	}


	// Public Interface Implementation
	inline virtual void updateComponents(const float& deltaTime, const std::vector< std::vector<BaseECSComponent*> >& components) override {
		const auto text = LightRadius_Component::STRING_NAME + ": (" + std::to_string(components.size()) + ")";
		if (ImGui::CollapsingHeader(text.c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
			auto radiusInput = ((LightRadius_Component*)components[0][1])->m_radius;
			if (ImGui::DragFloat("Radius", &radiusInput))
				for each (auto & componentParam in components)
					((LightRadius_Component*)componentParam[1])->m_radius = radiusInput;
		}
	}
};
#endif // INSPECTOR_LIGHTRADIUS_SYSTEM_H