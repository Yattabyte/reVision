#pragma once
#ifndef PLAYER2D_C_H
#define PLAYER2D_C_H

#include "Modules/World/ecsComponent.h"
#include "Utilities/Transform.h"
#include "glm/glm.hpp"


/** A component representing a basic player. */
struct Player2D_Component : public ECSComponent<Player2D_Component> {
	glm::vec3 m_rotation = glm::vec3(0.0f);
};
/** A constructor to aid in creation. */
struct Player2D_Constructor : ECSComponentConstructor<Player2D_Component> {
	// Interface Implementation
	inline virtual Component_and_ID construct(const std::vector<std::any> & parameters) override {
		auto * component = new Player2D_Component();
		return { component, component->ID };
	}
};

#endif // PLAYER2D_C_H