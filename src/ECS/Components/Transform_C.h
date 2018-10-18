#pragma once
#ifndef TRANSFORM_C_H
#define TRANSFORM_C_H

#include "ECS\Components\ecsComponent.h"
#include "Utilities\Transform.h"


/** A component representing a 3D spatial transformation. */
struct Transform_Component : public ECSComponent<Transform_Component> {
	Transform m_transform;
};
/** A constructor to aid in creation. */
struct Transform_Constructor : ECSComponentConstructor<Transform_Component> {
	// Interface Implementation
	virtual Component_and_ID construct(const std::vector<std::any> & parameters) override {
		auto position = castAny(parameters[0], glm::vec3(0.0f));
		auto orientation = castAny(parameters[1], glm::quat(1, 0, 0, 0));
		auto scale = castAny(parameters[2], glm::vec3(1.0f));

		auto * component = new Transform_Component();
		component->m_transform.m_position = position;
		component->m_transform.m_orientation = orientation;
		component->m_transform.m_scale = scale;
		component->m_transform.update();

		return { component, component->ID };
	}
};

#endif // BASICPLAYER_C_H