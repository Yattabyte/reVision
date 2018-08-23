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

#endif // BASICPLAYER_C_H