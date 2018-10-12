#pragma once
#ifndef PHYSICS_C_H
#define PHYSICS_C_H

#include "ECS\Components\ecsComponent.h"
#include "Utilities\Transform.h"
#include "glm\glm.hpp"


/** A component representing a basic player. */
struct Physics_Component : public ECSComponent<Physics_Component> {
};
/** A constructor to aid in creation. */
struct Physics_Constructor : ECSComponentConstructor<Physics_Component> {
	// Interface Implementation
	virtual Component_and_ID construct(const std::vector<std::any> & parameters) override {
		
		auto * component = new Physics_Component();
		
		return { component, component->ID };
	}
};

#endif // PHYSICS_C_H