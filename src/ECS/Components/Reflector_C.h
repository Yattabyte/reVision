#pragma once
#ifndef REFLECTOR_C_H
#define REFLECTOR_C_H

#include "ECS\Components\ecsComponent.h"
#include "ECS\Components\Camera_C.h"
#include "Utilities\GL\VectorBuffer.h"
#include "glm\glm.hpp"


/** OpenGL buffer for Parallax reflectors.
Formatted to adhere to glsl specifications. */
struct Reflection_Buffer {
	glm::mat4 mMatrix;
	glm::mat4 rotMatrix;
	glm::vec3 BoxCamPos; float padding1;
	glm::vec3 BoxScale; float padding2;
	int CubeSpot; glm::vec3 padding3;
};

/** Represents an environment map buffer component. */
struct Reflector_Component: public ECSComponent<Reflector_Component> {
	VB_Element<Reflection_Buffer> * m_data;
	int m_cubeSpot = 0;
	bool m_outOfDate = true;
	float m_updateTime = 0.0f;
	VB_Element<Camera_Buffer> * m_Cameradata[6];
};

#endif // REFLECTOR_C_H