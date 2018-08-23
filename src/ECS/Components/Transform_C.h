#pragma once
#ifndef TRANSFORM_C_H
#define TRANSFORM_C_H

#include "ECS\Components\ecsComponent.h"
#include "Utilities\Transform.h"


/** A 3D transformation structure, holding position, rotation, and scale. */
struct Transform_Component : public ECSComponent<Transform_Component> {	
	Transform m_transform;
};

#endif // TRANSFORM_C_H