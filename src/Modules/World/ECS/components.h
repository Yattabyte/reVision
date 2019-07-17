#pragma once
#ifndef WORLD_COMPONENTS_H
#define WORLD_COMPONENTS_H

#include "Modules/World/ECS/ecsComponent.h"
#include "Utilities/Transform.h"

/** A component representing a 3D spatial transformation. */
struct Transform_Component : public ECSComponent<Transform_Component> {
	Transform m_transform;
};

struct Bounding_Component : public ECSComponent<Bounding_Component> {

};

#endif // WORLD_COMPONENTS_H