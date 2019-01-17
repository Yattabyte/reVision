#pragma once
#ifndef PLAYER3D_C_H
#define PLAYER3D_C_H

#include "Utilities/ECS/ecsComponent.h"
#include "Utilities/Transform.h"
#include "glm/glm.hpp"


/** A component representing a basic player. */
struct Player3D_Component : public ECSComponent<Player3D_Component> {
	glm::vec3 m_rotation = glm::vec3(0.0f);
};
/** A constructor to aid in creation. */
struct Player3D_Constructor : ECSComponentConstructor<Player3D_Component> {
	// Interface Implementation
	inline virtual Component_and_ID construct(const std::vector<std::any> & parameters) override {
		auto * component = new Player3D_Component();
		return { component, component->ID };
	}
};

#endif // PLAYER3D_C_H