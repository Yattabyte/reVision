#pragma once
#ifndef BOUNDINGSPHERE_C_H
#define BOUNDINGSPHERE_C_H

#include "ECS\Components\ecsComponent.h"
#include "glm\glm.hpp"


/** A bounding sphere. */
struct BoundingSphere_Component : public ECSComponent<BoundingSphere_Component> {
	float m_radius = 0.0f;
	glm::vec3 m_position = glm::vec3(0.0f);
};
/** A constructor to aid in creation. */
struct BoundingSphere_Constructor : ECSComponentConstructor<BoundingSphere_Component>{
	// No Interface Override	
};

#endif // BOUNDINGSPHERE_C_H