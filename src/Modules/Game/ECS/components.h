#pragma once
#ifndef PLAYER3D_C_H
#define PLAYER3D_C_H

#include "Modules/World/ECS/ecsComponent.h"
#include "Utilities/Transform.h"
#include "glm/glm.hpp"


/** A component representing spawn point. */
struct PlayerSpawn_Component : public ECSComponent<PlayerSpawn_Component> {
};

/** A component representing a basic player. */
struct Player3D_Component : public ECSComponent<Player3D_Component> {
	glm::vec3 m_rotation = glm::vec3(0.0f);
};

#endif // PLAYER3D_C_H