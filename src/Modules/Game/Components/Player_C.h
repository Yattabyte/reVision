#pragma once
#ifndef BASICPLAYER_C_H
#define BASICPLAYER_C_H

#include "Utilities\ECS\ecsComponent.h"
#include "Utilities\Transform.h"
#include "glm\glm.hpp"


/** A component representing a basic player. */
struct Player_Component : public ECSComponent<Player_Component> {
	glm::vec3 m_rotation = glm::vec3(0.0f);
};
/** A constructor to aid in creation. */
struct Player_Constructor : ECSComponentConstructor<Player_Component> {
	// Interface Implementation
	inline virtual Component_and_ID construct(const std::vector<std::any> & parameters) override {
		auto * component = new Player_Component();
		return { component, component->ID };
	}
};

#endif // BASICPLAYER_C_H