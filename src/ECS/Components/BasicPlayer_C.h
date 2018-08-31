#pragma once
#ifndef BASICPLAYER_C_H
#define BASICPLAYER_C_H

#include "ECS\Components\ecsComponent.h"
#include "Utilities\Transform.h"
#include "glm\glm.hpp"


/** A component representing a basic player. */
struct BasicPlayer_Component : public ECSComponent<BasicPlayer_Component> {
	glm::vec3 m_rotation = glm::vec3(0.0f);
	Transform m_transform;
};
/** A constructor to aid in creation. */
struct BasicPlayer_Constructor : ECSComponentConstructor<BasicPlayer_Component> {
	// Interface Implementation
	virtual Component_and_ID construct(const std::vector<std::any> & parameters) override {
		auto position = castAny(parameters[0], glm::vec3(0.0f));
		auto * component = new BasicPlayer_Component();
		component->m_transform.m_position = position;
		component->m_transform.update();
		return { component, component->ID };
	}
};

#endif // BASICPLAYER_C_H