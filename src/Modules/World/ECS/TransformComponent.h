#pragma once
#ifndef TRANSFORM_COMPONENT_H
#define TRANSFORM_COMPONENT_H

#include "Modules/World/ECS/ecsComponent.h"
#include "Utilities/Transform.h"

/** A component representing a 3D spatial transformation. */
struct Transform_Component : public ECSComponent<Transform_Component> {
	Transform m_transform;
};

#endif // TRANSFORM_COMPONENT_H